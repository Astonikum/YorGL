#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include <vector>
#include "stb_truetype.h"

using Microsoft::WRL::ComPtr;

struct GlyphInfo {
    float u0, v0, u1, v1;
    float xoff, yoff;
    float width, height;
    float advance;
};

class SdfFontRenderer {
public:
    bool init(ID3D11Device* device, ID3D11DeviceContext* ctx,
              const unsigned char* ttfData, int ttfLen, float fontSize);
    void shutdown();

    ID3D11ShaderResourceView* getAtlasSRV() { return atlasSRV_.Get(); }
    int getAtlasWidth() const { return atlasW_; }
    int getAtlasHeight() const { return atlasH_; }

    const GlyphInfo* getGlyph(int codepoint) const;
    float getKerning(int leftCodepoint, int rightCodepoint) const;
    float getLineHeight() const { return lineHeight_; }
    float getAscent() const { return ascent_; }
    float getDescent() const { return descent_; }
    float getFontSize() const { return fontSize_; }

private:
    void bakeAtlas();

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;
    ComPtr<ID3D11ShaderResourceView> atlasSRV_;
    ComPtr<ID3D11Texture2D> atlasTex_;

    std::unordered_map<int, GlyphInfo> glyphs_;
    std::vector<unsigned char> fontData_;
    stbtt_fontinfo fontInfo_ = {};
    int atlasW_ = 0, atlasH_ = 0;
    float lineHeight_ = 0;
    float ascent_ = 0;
    float descent_ = 0;
    float fontSize_ = 0;
    float fontScale_ = 0;
    bool initialized_ = false;
};
