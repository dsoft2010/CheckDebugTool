#include <jni.h>
#include <string>
#include <unistd.h>
#include <android/log.h>

#define  LOG_TAG    "CheckDebug"
#define  LOGUNK(...)  __android_log_print(ANDROID_LOG_UNKNOWN,LOG_TAG,__VA_ARGS__)
#define  LOGDEF(...)  __android_log_print(ANDROID_LOG_DEFAULT,LOG_TAG,__VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGF(...)  __android_log_print(ANDROID_FATAL_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGS(...)  __android_log_print(ANDROID_SILENT_ERROR,LOG_TAG,__VA_ARGS__)

extern "C"
JNIEXPORT jboolean JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isDebugToolTracerPid(JNIEnv *env, jobject thiz) {
    int TPid;
    char buf[512];
    const char *str = "TracerPid:";

    LOGD("isDebugToolTracerPid enter");

    size_t strSize = strlen(str);
    FILE *file = fopen("/proc/self/status", "r");

    LOGD("isDebugToolTracerPid file = %p", file);

    while (fgets(buf, 512, file)) {
        if (!strncmp(buf, str, strSize)) {
            sscanf(buf, "TracerPid: %d", &TPid);
            if (TPid != 0) {
                fclose(file);
                LOGD("isDebugToolTracerPid true");
                return true;
            }
        }
    }

    fclose(file);
    LOGD("isDebugToolTracerPid false");
    return false;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isDebugToolCmdLine(JNIEnv *env, jobject thiz) {
    char filePath[32], fileRead[128];
    FILE *file;

    LOGD("isDebugToolCmdLine enter");
    snprintf(filePath, 24, "/proc/%d/cmdline", getppid());
    file = fopen(filePath, "r");
    if (file == nullptr) {
        LOGD("isDebugToolCmdLine nullptr false");
        return false;
    }
    fgets(fileRead, 128, file);
    fclose(file);

    if (!strcmp(fileRead, "gdb")) {
        LOGD("isDebugToolCmdLine gdb true");
        return true;
    }

    if (!strcmp(fileRead, "lldb")) {
        LOGD("isDebugToolCmdLine lldb true");
        return true;
    }

    LOGD("isDebugToolCmdLine false");
    return false;
}

const int version = 1;

extern "C"
JNIEXPORT jint JNICALL
Java_kr_ds_util_CheckDebugNativeLib_version(JNIEnv *env, jobject thiz) {
    LOGD("version = %d", version);
    return version;
}