#include "../yorgl/api.h"
#include <jni.h>

static YorGLRenderer* handle(jlong ptr) {
    return reinterpret_cast<YorGLRenderer*>(ptr);
}

extern "C" {

JNIEXPORT jlong JNICALL Java_org_yorgl_YorGLNative_create(JNIEnv*, jclass, jint backend) {
    return reinterpret_cast<jlong>(yorglCreate(static_cast<YorGLBackendKind>(backend)));
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_destroy(JNIEnv*, jclass, jlong ptr) {
    yorglDestroy(handle(ptr));
}

JNIEXPORT jboolean JNICALL Java_org_yorgl_YorGLNative_isValid(JNIEnv*, jclass, jlong ptr) {
    return yorglIsValid(handle(ptr)) != 0;
}

JNIEXPORT jstring JNICALL Java_org_yorgl_YorGLNative_backendName(JNIEnv* env, jclass, jlong ptr) {
    return env->NewStringUTF(yorglBackendName(handle(ptr)));
}

JNIEXPORT jintArray JNICALL Java_org_yorgl_YorGLNative_capabilities(JNIEnv* env, jclass, jlong ptr) {
    YorGLCapabilities caps{};
    yorglGetCapabilities(handle(ptr), &caps);
    jint values[7] = {
        caps.backend,
        caps.featureLevelMajor,
        caps.featureLevelMinor,
        caps.maxTextureSize,
        caps.presentVSync,
        caps.presentImmediate,
        caps.presentTearing,
    };
    jintArray result = env->NewIntArray(7);
    env->SetIntArrayRegion(result, 0, 7, values);
    return result;
}

JNIEXPORT jboolean JNICALL Java_org_yorgl_YorGLNative_createSwapChain(JNIEnv*, jclass, jlong ptr, jlong hwnd, jint width, jint height) {
    return yorglCreateSwapChain(handle(ptr), hwnd, width, height) != 0;
}

JNIEXPORT jboolean JNICALL Java_org_yorgl_YorGLNative_createSwapChainWithOptions(JNIEnv*, jclass, jlong ptr, jlong hwnd, jint width, jint height, jint bufferCount, jint presentMode, jboolean allowTearing) {
    YorGLSwapChainOptions options{};
    options.width = width;
    options.height = height;
    options.bufferCount = bufferCount;
    options.presentMode = static_cast<YorGLPresentMode>(presentMode);
    options.allowTearing = allowTearing ? 1 : 0;
    return yorglCreateSwapChainWithOptions(handle(ptr), hwnd, &options) != 0;
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_resize(JNIEnv*, jclass, jlong ptr, jint width, jint height) {
    yorglResize(handle(ptr), width, height);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_beginFrame(JNIEnv*, jclass, jlong ptr) {
    yorglBeginFrame(handle(ptr));
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_setViewport(JNIEnv*, jclass, jlong ptr, jfloat x, jfloat y, jfloat width, jfloat height) {
    yorglSetViewport(handle(ptr), x, y, width, height);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_clearColor(JNIEnv*, jclass, jlong ptr, jfloat r, jfloat g, jfloat b, jfloat a) {
    yorglClearColor(handle(ptr), r, g, b, a);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_clearDepth(JNIEnv*, jclass, jlong ptr, jfloat depth) {
    yorglClearDepth(handle(ptr), depth);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_setPresentMode(JNIEnv*, jclass, jlong ptr, jint mode) {
    yorglSetPresentMode(handle(ptr), static_cast<YorGLPresentMode>(mode));
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_endFrame(JNIEnv*, jclass, jlong ptr) {
    yorglEndFrame(handle(ptr));
}

JNIEXPORT jlong JNICALL Java_org_yorgl_YorGLNative_createTexture(JNIEnv* env, jclass, jlong ptr, jint width, jint height, jbyteArray pixels) {
    if (!pixels) return 0;
    jbyte* data = env->GetByteArrayElements(pixels, nullptr);
    jint len = env->GetArrayLength(pixels);
    jlong result = yorglCreateTexture(handle(ptr), width, height, reinterpret_cast<const uint8_t*>(data), len);
    env->ReleaseByteArrayElements(pixels, data, JNI_ABORT);
    return result;
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_destroyTexture(JNIEnv*, jclass, jlong ptr, jlong texture) {
    yorglDestroyTexture(handle(ptr), texture);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiBegin(JNIEnv*, jclass, jlong ptr, jint width, jint height) {
    yorglGuiBegin(handle(ptr), width, height);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiDrawQuad(JNIEnv*, jclass, jlong ptr, jfloat x, jfloat y, jfloat w, jfloat h, jfloat u0, jfloat v0, jfloat u1, jfloat v1, jfloat r, jfloat g, jfloat b, jfloat a) {
    yorglGuiDrawQuad(handle(ptr), x, y, w, h, u0, v0, u1, v1, r, g, b, a);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiDrawGradientQuad(JNIEnv* env, jclass, jlong ptr, jfloat x, jfloat y, jfloat w, jfloat h, jfloatArray rgba16) {
    if (!rgba16 || env->GetArrayLength(rgba16) < 16) return;
    jfloat* colors = env->GetFloatArrayElements(rgba16, nullptr);
    yorglGuiDrawGradientQuad(handle(ptr), x, y, w, h, colors);
    env->ReleaseFloatArrayElements(rgba16, colors, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiSetTexture(JNIEnv*, jclass, jlong ptr, jlong texture) {
    yorglGuiSetTexture(handle(ptr), texture);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiSetScissor(JNIEnv*, jclass, jlong ptr, jfloat x, jfloat y, jfloat w, jfloat h) {
    yorglGuiSetScissor(handle(ptr), x, y, w, h);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiClearScissor(JNIEnv*, jclass, jlong ptr) {
    yorglGuiClearScissor(handle(ptr));
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiSetSdfMode(JNIEnv*, jclass, jlong ptr, jboolean enabled) {
    yorglGuiSetSdfMode(handle(ptr), enabled);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiSetSdfParams(JNIEnv*, jclass, jlong ptr, jfloat edge, jfloat softness, jfloat weightBias) {
    yorglGuiSetSdfParams(handle(ptr), edge, softness, weightBias);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiBlurRect(JNIEnv*, jclass, jlong ptr, jfloat x, jfloat y, jfloat w, jfloat h, jint passes) {
    yorglGuiBlurRect(handle(ptr), x, y, w, h, passes);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_guiEnd(JNIEnv*, jclass, jlong ptr) {
    yorglGuiEnd(handle(ptr));
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_panoramaRender(JNIEnv* env, jclass, jlong ptr, jlongArray faces, jfloat angle, jint width, jint height) {
    if (!faces || env->GetArrayLength(faces) < 6) return;
    jlong* values = env->GetLongArrayElements(faces, nullptr);
    yorglPanoramaRender(handle(ptr), reinterpret_cast<const int64_t*>(values), angle, width, height);
    env->ReleaseLongArrayElements(faces, values, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_worldUploadMesh(JNIEnv* env, jclass, jlong ptr, jfloatArray vertices, jint floatCount) {
    if (!vertices || floatCount <= 0) {
        yorglWorldUploadMesh(handle(ptr), nullptr, 0);
        return;
    }
    jfloat* data = env->GetFloatArrayElements(vertices, nullptr);
    jint len = env->GetArrayLength(vertices);
    yorglWorldUploadMesh(handle(ptr), data, floatCount < len ? floatCount : len);
    env->ReleaseFloatArrayElements(vertices, data, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_worldUploadSection(JNIEnv* env, jclass, jlong ptr, jlong sectionId, jint x, jint y, jint z, jfloatArray vertices, jint floatCount) {
    if (!vertices || floatCount <= 0) {
        yorglWorldUploadSection(handle(ptr), sectionId, x, y, z, nullptr, 0);
        return;
    }
    jfloat* data = env->GetFloatArrayElements(vertices, nullptr);
    jint len = env->GetArrayLength(vertices);
    yorglWorldUploadSection(handle(ptr), sectionId, x, y, z, data, floatCount < len ? floatCount : len);
    env->ReleaseFloatArrayElements(vertices, data, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_worldUploadSectionLayer(JNIEnv* env, jclass, jlong ptr, jlong sectionId, jint x, jint y, jint z, jint layer, jfloatArray vertices, jint floatCount) {
    if (!vertices || floatCount <= 0) {
        yorglWorldUploadSectionLayer(handle(ptr), sectionId, x, y, z, layer, nullptr, 0);
        return;
    }
    jfloat* data = env->GetFloatArrayElements(vertices, nullptr);
    jint len = env->GetArrayLength(vertices);
    yorglWorldUploadSectionLayer(handle(ptr), sectionId, x, y, z, layer, data, floatCount < len ? floatCount : len);
    env->ReleaseFloatArrayElements(vertices, data, JNI_ABORT);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_worldRemoveSection(JNIEnv*, jclass, jlong ptr, jlong sectionId) {
    yorglWorldRemoveSection(handle(ptr), sectionId);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_worldClearSections(JNIEnv*, jclass, jlong ptr) {
    yorglWorldClearSections(handle(ptr));
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_worldSetTexture(JNIEnv*, jclass, jlong ptr, jlong texture) {
    yorglWorldSetTexture(handle(ptr), texture);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_worldSetSkyColor(JNIEnv*, jclass, jlong ptr, jfloat r, jfloat g, jfloat b) {
    yorglWorldSetSkyColor(handle(ptr), r, g, b);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_worldRender(JNIEnv*, jclass, jlong ptr, jfloat cameraX, jfloat cameraY, jfloat cameraZ, jfloat dirX, jfloat dirY, jfloat dirZ, jfloat fovYDegrees, jfloat farPlane, jint width, jint height) {
    yorglWorldRender(handle(ptr), cameraX, cameraY, cameraZ, dirX, dirY, dirZ, fovYDegrees, farPlane, width, height);
}

JNIEXPORT jlong JNICALL Java_org_yorgl_YorGLNative_sdfFontCreate(JNIEnv* env, jclass, jlong ptr, jbyteArray ttfData, jfloat fontSize) {
    if (!ttfData) return 0;
    jbyte* data = env->GetByteArrayElements(ttfData, nullptr);
    jint len = env->GetArrayLength(ttfData);
    jlong result = yorglSdfFontCreate(handle(ptr), reinterpret_cast<const uint8_t*>(data), len, fontSize);
    env->ReleaseByteArrayElements(ttfData, data, JNI_ABORT);
    return result;
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_sdfFontDestroy(JNIEnv*, jclass, jlong ptr, jlong font) {
    yorglSdfFontDestroy(handle(ptr), font);
}

JNIEXPORT jlong JNICALL Java_org_yorgl_YorGLNative_sdfFontAtlas(JNIEnv*, jclass, jlong ptr, jlong font) {
    return yorglSdfFontAtlas(handle(ptr), font);
}

JNIEXPORT jfloatArray JNICALL Java_org_yorgl_YorGLNative_sdfFontMetrics(JNIEnv* env, jclass, jlong ptr, jlong font) {
    float out[3]{};
    if (!yorglSdfFontMetrics(handle(ptr), font, out)) return nullptr;
    jfloatArray result = env->NewFloatArray(3);
    env->SetFloatArrayRegion(result, 0, 3, out);
    return result;
}

JNIEXPORT jfloatArray JNICALL Java_org_yorgl_YorGLNative_sdfFontGlyph(JNIEnv* env, jclass, jlong ptr, jlong font, jint codepoint) {
    float out[9]{};
    if (!yorglSdfFontGlyph(handle(ptr), font, codepoint, out)) return nullptr;
    jfloatArray result = env->NewFloatArray(9);
    env->SetFloatArrayRegion(result, 0, 9, out);
    return result;
}

JNIEXPORT jfloat JNICALL Java_org_yorgl_YorGLNative_sdfFontKerning(JNIEnv*, jclass, jlong ptr, jlong font, jint leftCodepoint, jint rightCodepoint) {
    return yorglSdfFontKerning(handle(ptr), font, leftCodepoint, rightCodepoint);
}

JNIEXPORT jfloat JNICALL Java_org_yorgl_YorGLNative_sdfFontTextWidth(JNIEnv* env, jclass, jlong ptr, jlong font, jstring text, jfloat scale) {
    if (!text) return 0.0f;
    const char* utf8 = env->GetStringUTFChars(text, nullptr);
    jsize len = env->GetStringUTFLength(text);
    jfloat result = yorglSdfFontTextWidth(handle(ptr), font, utf8, len, scale);
    env->ReleaseStringUTFChars(text, utf8);
    return result;
}

JNIEXPORT jfloat JNICALL Java_org_yorgl_YorGLNative_sdfFontLineHeight(JNIEnv*, jclass, jlong ptr, jlong font, jfloat scale) {
    return yorglSdfFontLineHeight(handle(ptr), font, scale);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_sdfFontDrawText(JNIEnv* env, jclass, jlong ptr, jlong font, jstring text, jfloat x, jfloat y, jfloat scale, jfloat r, jfloat g, jfloat b, jfloat a, jfloat weight, jboolean shadow) {
    if (!text) return;
    const char* utf8 = env->GetStringUTFChars(text, nullptr);
    jsize len = env->GetStringUTFLength(text);
    yorglSdfFontDrawText(handle(ptr), font, utf8, len, x, y, scale, r, g, b, a, weight, shadow);
    env->ReleaseStringUTFChars(text, utf8);
}

}
