#include <jni.h>
#include "dx11_backend.h"
#include "sdf_font.h"
#include "frost_log.h"

static DX11Backend* getBackend(jlong ptr) {
    return reinterpret_cast<DX11Backend*>(ptr);
}

static SdfFontRenderer* g_sdfFont = nullptr;

extern "C" {

JNIEXPORT jlong JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nInit(JNIEnv*, jclass) {
    LOG_INFO("JNI: nInit called");
    auto* backend = new DX11Backend();
    if (!backend->init()) {
        LOG_ERROR("JNI: Backend init failed, returning null");
        delete backend;
        return 0;
    }
    LOG_INFO("JNI: Backend initialized at %p", backend);
    return reinterpret_cast<jlong>(backend);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nShutdown(JNIEnv*, jclass, jlong ptr) {
    LOG_INFO("JNI: nShutdown");
    auto* b = getBackend(ptr);
    b->shutdown();
    delete b;
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nBeginFrame(JNIEnv*, jclass, jlong ptr) {
    getBackend(ptr)->beginFrame();
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nEndFrame(JNIEnv*, jclass, jlong ptr) {
    getBackend(ptr)->endFrame();
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nCreateSwapChain(
    JNIEnv*, jclass, jlong ptr, jlong hwnd, jint w, jint h) {
    LOG_INFO("JNI: createSwapChain (hwnd=0x%llX, %dx%d)", hwnd, w, h);
    getBackend(ptr)->createSwapChain(hwnd, w, h);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nResizeSwapChain(
    JNIEnv*, jclass, jlong ptr, jint w, jint h) {
    getBackend(ptr)->resizeSwapChain(w, h);
}

JNIEXPORT jlong JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nCreateShader(
    JNIEnv* env, jclass, jlong ptr, jbyteArray vs, jbyteArray ps) {
    jbyte* vsData = env->GetByteArrayElements(vs, nullptr);
    jint vsLen = env->GetArrayLength(vs);
    jbyte* psData = env->GetByteArrayElements(ps, nullptr);
    jint psLen = env->GetArrayLength(ps);
    jlong result = getBackend(ptr)->createShader((uint8_t*)vsData, vsLen, (uint8_t*)psData, psLen);
    env->ReleaseByteArrayElements(vs, vsData, JNI_ABORT);
    env->ReleaseByteArrayElements(ps, psData, JNI_ABORT);
    return result;
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nDestroyShader(
    JNIEnv*, jclass, jlong ptr, jlong handle) {
    getBackend(ptr)->destroyShader(handle);
}

JNIEXPORT jlong JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nCreateBuffer(
    JNIEnv* env, jclass, jlong ptr, jint type, jbyteArray data, jint usage) {
    jbyte* raw = env->GetByteArrayElements(data, nullptr);
    jint len = env->GetArrayLength(data);
    jlong result = getBackend(ptr)->createBuffer(type, (uint8_t*)raw, len, usage);
    env->ReleaseByteArrayElements(data, raw, JNI_ABORT);
    return result;
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nDestroyBuffer(
    JNIEnv*, jclass, jlong ptr, jlong handle) {
    getBackend(ptr)->destroyBuffer(handle);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nDraw(
    JNIEnv*, jclass, jlong ptr, jint vertexCount, jint startVertex) {
    getBackend(ptr)->draw(vertexCount, startVertex);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nDrawIndexed(
    JNIEnv*, jclass, jlong ptr, jint indexCount, jint startIndex, jint baseVertex) {
    getBackend(ptr)->drawIndexed(indexCount, startIndex, baseVertex);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nSetViewport(
    JNIEnv*, jclass, jlong ptr, jfloat x, jfloat y, jfloat w, jfloat h) {
    getBackend(ptr)->setViewport(x, y, w, h);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nClearColor(
    JNIEnv*, jclass, jlong ptr, jfloat r, jfloat g, jfloat b, jfloat a) {
    getBackend(ptr)->clearColor(r, g, b, a);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nClearDepth(
    JNIEnv*, jclass, jlong ptr, jfloat depth) {
    getBackend(ptr)->clearDepth(depth);
}

// --- GUI Renderer JNI ---

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiBegin(
    JNIEnv*, jclass, jlong ptr, jint w, jint h) {
    getBackend(ptr)->gui().begin(w, h);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiDrawQuad(
    JNIEnv*, jclass, jlong ptr,
    jfloat x, jfloat y, jfloat w, jfloat h,
    jfloat u0, jfloat v0, jfloat u1, jfloat v1,
    jfloat r, jfloat g, jfloat b, jfloat a) {
    getBackend(ptr)->gui().drawQuad(x, y, w, h, u0, v0, u1, v1, r, g, b, a);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiDrawGradientQuad(
    JNIEnv*, jclass, jlong ptr,
    jfloat x, jfloat y, jfloat w, jfloat h,
    jfloat tlR, jfloat tlG, jfloat tlB, jfloat tlA,
    jfloat trR, jfloat trG, jfloat trB, jfloat trA,
    jfloat brR, jfloat brG, jfloat brB, jfloat brA,
    jfloat blR, jfloat blG, jfloat blB, jfloat blA) {
    getBackend(ptr)->gui().drawGradientQuad(x, y, w, h,
        tlR, tlG, tlB, tlA,
        trR, trG, trB, trA,
        brR, brG, brB, brA,
        blR, blG, blB, blA);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiEnd(
    JNIEnv*, jclass, jlong ptr) {
    getBackend(ptr)->gui().end();
}

JNIEXPORT jlong JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nCreateTexture(
    JNIEnv* env, jclass, jlong ptr, jint width, jint height, jbyteArray pixels) {
    jbyte* data = env->GetByteArrayElements(pixels, nullptr);
    jint len = env->GetArrayLength(pixels);
    LOG_INFO("JNI: createTexture %dx%d (%d bytes)", width, height, len);

    auto* backend = getBackend(ptr);
    ID3D11Device* device = backend->getDevice();

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = data;
    sd.SysMemPitch = width * 4;

    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = device->CreateTexture2D(&td, &sd, &tex);
    env->ReleaseByteArrayElements(pixels, data, JNI_ABORT);

    if (FAILED(hr)) {
        LOG_ERROR("CreateTexture2D failed: 0x%08X", hr);
        return 0;
    }

    ID3D11ShaderResourceView* srv = nullptr;
    hr = device->CreateShaderResourceView(tex, nullptr, &srv);
    tex->Release();

    if (FAILED(hr)) {
        LOG_ERROR("CreateSRV failed: 0x%08X", hr);
        return 0;
    }

    LOG_INFO("Texture created: %dx%d, SRV=%p", width, height, srv);
    return reinterpret_cast<jlong>(srv);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiSetTexture(
    JNIEnv*, jclass, jlong ptr, jlong texHandle) {
    auto* srv = reinterpret_cast<ID3D11ShaderResourceView*>(texHandle);
    getBackend(ptr)->gui().setTexture(srv);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nDestroyTexture(
    JNIEnv*, jclass, jlong texHandle) {
    auto* srv = reinterpret_cast<ID3D11ShaderResourceView*>(texHandle);
    if (srv) srv->Release();
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiSetScissor(
    JNIEnv*, jclass, jlong ptr, jfloat x, jfloat y, jfloat w, jfloat h) {
    getBackend(ptr)->gui().setScissor(x, y, w, h);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiClearScissor(
    JNIEnv*, jclass, jlong ptr) {
    getBackend(ptr)->gui().clearScissor();
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiSetSdfMode(
    JNIEnv*, jclass, jlong ptr, jboolean sdf) {
    getBackend(ptr)->gui().setSdfMode(sdf);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiSetSdfParams(
    JNIEnv*, jclass, jlong ptr, jfloat edge, jfloat softness, jfloat weightBias) {
    getBackend(ptr)->gui().setSdfParams(edge, softness, weightBias);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nGuiBlurRect(
    JNIEnv*, jclass, jlong ptr, jfloat x, jfloat y, jfloat w, jfloat h, jint passes) {
    auto* backend = getBackend(ptr);
    backend->gui().blurRect(x, y, w, h, passes, backend->getRTV());
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nPanoramaRender(
    JNIEnv*, jclass, jlong ptr,
    jlong f0, jlong f1, jlong f2, jlong f3, jlong f4, jlong f5,
    jfloat angle, jint screenW, jint screenH) {
    auto* backend = getBackend(ptr);
    ID3D11ShaderResourceView* faces[6] = {
        reinterpret_cast<ID3D11ShaderResourceView*>(f0),
        reinterpret_cast<ID3D11ShaderResourceView*>(f1),
        reinterpret_cast<ID3D11ShaderResourceView*>(f2),
        reinterpret_cast<ID3D11ShaderResourceView*>(f3),
        reinterpret_cast<ID3D11ShaderResourceView*>(f4),
        reinterpret_cast<ID3D11ShaderResourceView*>(f5)
    };
    backend->gui().flush();
    backend->panorama().render(faces, angle, screenW, screenH, backend->getRTV());
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nWorldUploadMesh(
    JNIEnv* env, jclass, jlong ptr, jfloatArray vertices, jint floatCount) {
    auto* backend = getBackend(ptr);
    if (!vertices || floatCount <= 0) {
        backend->world().uploadMesh(nullptr, 0);
        return;
    }
    jfloat* data = env->GetFloatArrayElements(vertices, nullptr);
    jint len = env->GetArrayLength(vertices);
    int safeCount = floatCount < len ? floatCount : len;
    backend->world().uploadMesh((const float*)data, safeCount);
    env->ReleaseFloatArrayElements(vertices, data, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nWorldUploadSection(
    JNIEnv* env, jclass, jlong ptr, jlong sectionId, jint sectionX, jint sectionY, jint sectionZ, jfloatArray vertices, jint floatCount) {
    auto* backend = getBackend(ptr);
    if (!vertices || floatCount <= 0) {
        backend->world().removeSection(sectionId);
        return;
    }
    jfloat* data = env->GetFloatArrayElements(vertices, nullptr);
    jint len = env->GetArrayLength(vertices);
    int safeCount = floatCount < len ? floatCount : len;
    backend->world().uploadSection(sectionId, sectionX, sectionY, sectionZ, (const float*)data, safeCount);
    env->ReleaseFloatArrayElements(vertices, data, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nWorldUploadSectionLayer(
    JNIEnv* env, jclass, jlong ptr, jlong sectionId, jint sectionX, jint sectionY, jint sectionZ, jint layer, jfloatArray vertices, jint floatCount) {
    auto* backend = getBackend(ptr);
    if (!vertices || floatCount <= 0) {
        backend->world().uploadSectionLayer(sectionId, sectionX, sectionY, sectionZ, layer, nullptr, 0);
        return;
    }
    jfloat* data = env->GetFloatArrayElements(vertices, nullptr);
    jint len = env->GetArrayLength(vertices);
    int safeCount = floatCount < len ? floatCount : len;
    backend->world().uploadSectionLayer(sectionId, sectionX, sectionY, sectionZ, layer, (const float*)data, safeCount);
    env->ReleaseFloatArrayElements(vertices, data, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nWorldRemoveSection(
    JNIEnv*, jclass, jlong ptr, jlong sectionId) {
    getBackend(ptr)->world().removeSection(sectionId);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nWorldClearSections(
    JNIEnv*, jclass, jlong ptr) {
    getBackend(ptr)->world().clearSections();
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nWorldSetTexture(
    JNIEnv*, jclass, jlong ptr, jlong texHandle) {
    auto* srv = reinterpret_cast<ID3D11ShaderResourceView*>(texHandle);
    getBackend(ptr)->world().setTexture(srv);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nWorldSetSkyColor(
    JNIEnv*, jclass, jlong ptr, jfloat r, jfloat g, jfloat b) {
    getBackend(ptr)->world().setSkyColor(r, g, b);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nWorldRender(
    JNIEnv*, jclass, jlong ptr,
    jfloat cameraX, jfloat cameraY, jfloat cameraZ,
    jfloat dirX, jfloat dirY, jfloat dirZ, jfloat fovYDegrees, jfloat farPlane, jint screenW, jint screenH) {
    auto* backend = getBackend(ptr);
    backend->gui().flush();
    backend->world().render(cameraX, cameraY, cameraZ, dirX, dirY, dirZ, fovYDegrees, farPlane, screenW, screenH,
                            backend->getRTV(), backend->getDSV());
}

// --- SDF Font JNI ---

JNIEXPORT jlong JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nSdfFontInit(
    JNIEnv* env, jclass, jlong ptr, jbyteArray ttfData, jfloat fontSize) {
    auto* backend = getBackend(ptr);
    jbyte* data = env->GetByteArrayElements(ttfData, nullptr);
    jint len = env->GetArrayLength(ttfData);

    auto* font = new SdfFontRenderer();
    if (!font->init(backend->getDevice(), backend->getContext(),
                    (const unsigned char*)data, len, fontSize)) {
        delete font;
        env->ReleaseByteArrayElements(ttfData, data, JNI_ABORT);
        return 0;
    }
    env->ReleaseByteArrayElements(ttfData, data, JNI_ABORT);
    g_sdfFont = font;
    return reinterpret_cast<jlong>(font);
}

JNIEXPORT void JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nSdfFontShutdown(
    JNIEnv*, jclass, jlong fontPtr) {
    auto* font = reinterpret_cast<SdfFontRenderer*>(fontPtr);
    if (font) { font->shutdown(); delete font; }
    if (g_sdfFont == font) g_sdfFont = nullptr;
}

JNIEXPORT jlong JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nSdfFontGetAtlas(
    JNIEnv*, jclass, jlong fontPtr) {
    auto* font = reinterpret_cast<SdfFontRenderer*>(fontPtr);
    return reinterpret_cast<jlong>(font->getAtlasSRV());
}

JNIEXPORT jfloatArray JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nSdfFontGetMetrics(
    JNIEnv* env, jclass, jlong fontPtr) {
    auto* font = reinterpret_cast<SdfFontRenderer*>(fontPtr);
    jfloatArray arr = env->NewFloatArray(3);
    float metrics[3] = {font->getLineHeight(), font->getAscent(), font->getDescent()};
    env->SetFloatArrayRegion(arr, 0, 3, metrics);
    return arr;
}

JNIEXPORT jfloatArray JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nSdfFontGetGlyph(
    JNIEnv* env, jclass, jlong fontPtr, jint codepoint) {
    auto* font = reinterpret_cast<SdfFontRenderer*>(fontPtr);
    const GlyphInfo* g = font->getGlyph(codepoint);
    if (!g) return nullptr;
    jfloatArray arr = env->NewFloatArray(9);
    float data[9] = {g->u0, g->v0, g->u1, g->v1, g->xoff, g->yoff, g->width, g->height, g->advance};
    env->SetFloatArrayRegion(arr, 0, 9, data);
    return arr;
}

JNIEXPORT jfloat JNICALL Java_dev_frost_engine_render_backend_NativeBridgeKt_nSdfFontGetKerning(
    JNIEnv*, jclass, jlong fontPtr, jint leftCodepoint, jint rightCodepoint) {
    auto* font = reinterpret_cast<SdfFontRenderer*>(fontPtr);
    if (!font) return 0.0f;
    return font->getKerning(leftCodepoint, rightCodepoint);
}

} // extern "C"
