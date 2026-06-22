#include "sdf_font.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "frost_log.h"
#include <cstring>
#include <algorithm>

static const int SDF_PADDING = 5;
static const int SDF_ONEDGE = 180;
static const float SDF_PIXEL_DIST_SCALE = 180.0f / (float)SDF_PADDING;

bool SdfFontRenderer::init(ID3D11Device* device, ID3D11DeviceContext* ctx,
                           const unsigned char* ttfData, int ttfLen, float fontSize) {
    device_ = device;
    ctx_ = ctx;
    fontSize_ = fontSize;
    fontData_.assign(ttfData, ttfData + ttfLen);
    if (!stbtt_InitFont(&fontInfo_, fontData_.data(), stbtt_GetFontOffsetForIndex(fontData_.data(), 0))) {
        LOG_ERROR("SdfFont: failed to init font");
        return false;
    }
    fontScale_ = stbtt_ScaleForPixelHeight(&fontInfo_, fontSize);
    bakeAtlas(ttfData, ttfLen, fontSize);
    initialized_ = (atlasSRV_ != nullptr);
    return initialized_;
}

void SdfFontRenderer::shutdown() {
    atlasSRV_.Reset();
    atlasTex_.Reset();
    glyphs_.clear();
    fontData_.clear();
    fontInfo_ = {};
    fontScale_ = 0;
    initialized_ = false;
}

const GlyphInfo* SdfFontRenderer::getGlyph(int codepoint) const {
    auto it = glyphs_.find(codepoint);
    return it != glyphs_.end() ? &it->second : nullptr;
}

float SdfFontRenderer::getKerning(int leftCodepoint, int rightCodepoint) const {
    if (!initialized_ || fontScale_ <= 0.0f || leftCodepoint == 0 || rightCodepoint == 0) return 0.0f;
    return stbtt_GetCodepointKernAdvance(&fontInfo_, leftCodepoint, rightCodepoint) * fontScale_;
}

void SdfFontRenderer::bakeAtlas(const unsigned char* ttfData, int ttfLen, float fontSize) {
    stbtt_fontinfo& font = fontInfo_;
    float scale = fontScale_;
    int iAscent, iDescent, iLineGap;
    stbtt_GetFontVMetrics(&font, &iAscent, &iDescent, &iLineGap);
    ascent_ = iAscent * scale;
    descent_ = iDescent * scale;
    lineHeight_ = (iAscent - iDescent + iLineGap) * scale;

    // Determine atlas size: pack ASCII 32-126 + Cyrillic 0x400-0x4FF
    struct CharRange { int start; int end; };
    CharRange ranges[] = {{32, 127}, {0x400, 0x500}};
    int totalGlyphs = 0;
    for (auto& r : ranges) totalGlyphs += r.end - r.start;

    atlasW_ = 1024;
    atlasH_ = 1024;
    std::vector<unsigned char> atlas(atlasW_ * atlasH_, 0);

    int cursorX = 0, cursorY = 0, rowH = 0;

    for (auto& range : ranges) {
        for (int cp = range.start; cp < range.end; cp++) {
            int glyph = stbtt_FindGlyphIndex(&font, cp);
            if (glyph == 0 && cp != 32) continue;

            int w, h, xoff, yoff;
            unsigned char* sdf = stbtt_GetGlyphSDF(&font, scale, glyph,
                SDF_PADDING, SDF_ONEDGE, SDF_PIXEL_DIST_SCALE, &w, &h, &xoff, &yoff);

            if (!sdf) {
                int advW, lsb;
                stbtt_GetGlyphHMetrics(&font, glyph, &advW, &lsb);
                GlyphInfo gi = {0,0,0,0, 0,0, 0,0, advW * scale};
                glyphs_[cp] = gi;
                continue;
            }

            if (cursorX + w > atlasW_) {
                cursorX = 0;
                cursorY += rowH + 1;
                rowH = 0;
            }
            if (cursorY + h > atlasH_) {
                stbtt_FreeSDF(sdf, nullptr);
                LOG_ERROR("SdfFont: atlas overflow at codepoint %d", cp);
                break;
            }

            for (int row = 0; row < h; row++)
                memcpy(&atlas[(cursorY + row) * atlasW_ + cursorX], &sdf[row * w], w);

            float u0 = (float)cursorX / atlasW_;
            float v0 = (float)cursorY / atlasH_;
            float u1 = (float)(cursorX + w) / atlasW_;
            float v1 = (float)(cursorY + h) / atlasH_;

            int advW, lsb;
            stbtt_GetGlyphHMetrics(&font, glyph, &advW, &lsb);

            GlyphInfo gi = {u0, v0, u1, v1,
                            (float)xoff, (float)yoff,
                            (float)w, (float)h,
                            advW * scale};
            glyphs_[cp] = gi;

            cursorX += w + 1;
            rowH = std::max(rowH, h);
            stbtt_FreeSDF(sdf, nullptr);
        }
    }

    LOG_INFO("SdfFont: baked %d glyphs, atlas %dx%d", (int)glyphs_.size(), atlasW_, atlasH_);

    // Convert R8 atlas to R8G8B8A8 (white RGB + SDF in alpha) for gui shader compatibility
    std::vector<unsigned char> rgba(atlasW_ * atlasH_ * 4);
    for (int i = 0; i < atlasW_ * atlasH_; i++) {
        rgba[i * 4 + 0] = 255;
        rgba[i * 4 + 1] = 255;
        rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = atlas[i];
    }

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = atlasW_;
    td.Height = atlasH_;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = rgba.data();
    sd.SysMemPitch = atlasW_ * 4;

    HRESULT hr = device_->CreateTexture2D(&td, &sd, &atlasTex_);
    if (FAILED(hr)) { LOG_ERROR("SdfFont: CreateTexture2D failed"); return; }

    device_->CreateShaderResourceView(atlasTex_.Get(), nullptr, &atlasSRV_);
    LOG_INFO("SdfFont: atlas texture created, SRV=%p", atlasSRV_.Get());
}
