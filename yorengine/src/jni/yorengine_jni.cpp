#include "yorengine/api.h"

#include <jni.h>

#include <cstdint>

namespace {

YorEngineScene* scene(jlong handle) {
    return reinterpret_cast<YorEngineScene*>(static_cast<uintptr_t>(handle));
}

YorEngineEntityId entity(jlong packed) {
    return {
        static_cast<uint32_t>(static_cast<uint64_t>(packed) & 0xffffffffULL),
        static_cast<uint32_t>(static_cast<uint64_t>(packed) >> 32),
    };
}

jlong pack(YorEngineEntityId value) {
    const uint64_t packed = (static_cast<uint64_t>(value.generation) << 32) | value.index;
    return static_cast<jlong>(packed);
}

void throwStatus(JNIEnv* env, YorEngineStatus status) {
    if (status == YORENGINE_STATUS_OK) return;

    jclass exception = env->FindClass("org/yorengine/NativeSceneException");
    if (!exception) return;
    jmethodID constructor = env->GetMethodID(exception, "<init>", "(ILjava/lang/String;)V");
    if (!constructor) return;
    const char* error = yorengineLastError();
    jstring message = env->NewStringUTF(error ? error : yorengineStatusName(status));
    if (!message) return;
    jobject value = env->NewObject(exception, constructor, static_cast<jint>(status), message);
    env->DeleteLocalRef(message);
    if (value) {
        env->Throw(static_cast<jthrowable>(value));
        env->DeleteLocalRef(value);
    }
}

bool readFloats(JNIEnv* env, jfloatArray source, jsize expected, float* destination) {
    if (!source || env->GetArrayLength(source) != expected) {
        jclass exception = env->FindClass("java/lang/IllegalArgumentException");
        if (exception) env->ThrowNew(exception, "Unexpected native value array length");
        return false;
    }
    jfloat* values = env->GetFloatArrayElements(source, nullptr);
    if (!values) return false;
    for (jsize index = 0; index < expected; ++index) destination[index] = values[index];
    env->ReleaseFloatArrayElements(source, values, JNI_ABORT);
    return true;
}

} // namespace

extern "C" {

JNIEXPORT jlong JNICALL Java_org_yorengine_NativeScene_nativeCreate(JNIEnv*, jclass) {
    return reinterpret_cast<jlong>(yorengineSceneCreate());
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeDestroy(JNIEnv*, jclass, jlong handle) {
    yorengineSceneDestroy(scene(handle));
}

JNIEXPORT jlong JNICALL Java_org_yorengine_NativeScene_nativeCreateEntity(JNIEnv* env, jclass, jlong handle) {
    YorEngineEntityId value{};
    throwStatus(env, yorengineSceneCreateEntity(scene(handle), &value));
    return pack(value);
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeDestroyEntity(JNIEnv* env, jclass, jlong handle, jlong packedEntity) {
    throwStatus(env, yorengineSceneDestroyEntity(scene(handle), entity(packedEntity)));
}

JNIEXPORT jboolean JNICALL Java_org_yorengine_NativeScene_nativeIsAlive(JNIEnv*, jclass, jlong handle, jlong packedEntity) {
    return yorengineSceneIsAlive(scene(handle), entity(packedEntity)) != 0;
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeSetParent(JNIEnv* env, jclass, jlong handle, jlong packedChild, jlong packedParent) {
    throwStatus(env, yorengineSceneSetParent(scene(handle), entity(packedChild), entity(packedParent)));
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeSetTransform(JNIEnv* env, jclass, jlong handle, jlong packedEntity, jfloatArray values) {
    float data[10]{};
    if (!readFloats(env, values, 10, data)) return;
    const YorEngineTransform transform{
        {data[0], data[1], data[2]},
        {data[3], data[4], data[5], data[6]},
        {data[7], data[8], data[9]},
    };
    throwStatus(env, yorengineSceneSetTransform(scene(handle), entity(packedEntity), &transform));
}

JNIEXPORT jfloatArray JNICALL Java_org_yorengine_NativeScene_nativeWorldMatrix(JNIEnv* env, jclass, jlong handle, jlong packedEntity) {
    YorEngineMat4 matrix{};
    const YorEngineStatus status = yorengineSceneGetWorldMatrix(scene(handle), entity(packedEntity), &matrix);
    if (status != YORENGINE_STATUS_OK) {
        throwStatus(env, status);
        return nullptr;
    }
    jfloatArray result = env->NewFloatArray(16);
    if (!result) return nullptr;
    env->SetFloatArrayRegion(result, 0, 16, matrix.values);
    return result;
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeAddMesh(JNIEnv* env, jclass, jlong handle, jlong packedEntity) {
    throwStatus(env, yorengineSceneAddMesh(scene(handle), entity(packedEntity)));
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeAddCamera(JNIEnv* env, jclass, jlong handle, jlong packedEntity) {
    throwStatus(env, yorengineSceneAddCamera(scene(handle), entity(packedEntity)));
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeSetCamera(JNIEnv* env, jclass, jlong handle, jlong packedEntity, jfloat fov, jfloat aspect, jfloat nearPlane, jfloat farPlane) {
    const YorEngineCameraState camera{fov, aspect, nearPlane, farPlane};
    throwStatus(env, yorengineSceneSetCamera(scene(handle), entity(packedEntity), &camera));
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeAddLight(JNIEnv* env, jclass, jlong handle, jlong packedEntity, jint kind) {
    throwStatus(env, yorengineSceneAddLight(scene(handle), entity(packedEntity), static_cast<YorEngineLightKind>(kind)));
}

JNIEXPORT void JNICALL Java_org_yorengine_NativeScene_nativeSetLight(JNIEnv* env, jclass, jlong handle, jlong packedEntity, jint kind, jfloat red, jfloat green, jfloat blue, jfloat intensity, jfloat range, jfloat innerCone, jfloat outerCone) {
    const YorEngineLightState light{
        static_cast<YorEngineLightKind>(kind),
        {red, green, blue},
        intensity,
        range,
        innerCone,
        outerCone,
    };
    throwStatus(env, yorengineSceneSetLight(scene(handle), entity(packedEntity), &light));
}

}
