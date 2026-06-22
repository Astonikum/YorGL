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

JNIEXPORT jboolean JNICALL Java_org_yorgl_YorGLNative_createSwapChain(JNIEnv*, jclass, jlong ptr, jlong hwnd, jint width, jint height) {
    return yorglCreateSwapChain(handle(ptr), hwnd, width, height) != 0;
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_resize(JNIEnv*, jclass, jlong ptr, jint width, jint height) {
    yorglResize(handle(ptr), width, height);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_beginFrame(JNIEnv*, jclass, jlong ptr) {
    yorglBeginFrame(handle(ptr));
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_clearColor(JNIEnv*, jclass, jlong ptr, jfloat r, jfloat g, jfloat b, jfloat a) {
    yorglClearColor(handle(ptr), r, g, b, a);
}

JNIEXPORT void JNICALL Java_org_yorgl_YorGLNative_endFrame(JNIEnv*, jclass, jlong ptr) {
    yorglEndFrame(handle(ptr));
}

}
