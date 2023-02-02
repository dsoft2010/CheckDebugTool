#include <jni.h>
#include <string>
#include <unistd.h>

extern "C"
JNIEXPORT jboolean JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isDebugToolTracerPid(JNIEnv *env, jobject thiz) {
    int TPid;
    char buf[512];
    const char *str = "TracerPid:";
    size_t strSize = strlen(str);
    FILE *file = fopen("/proc/self/status", "r");

    while (fgets(buf, 512, file)) {
        if (!strncmp(buf, str, strSize)) {
            sscanf(buf, "TracerPid: %d", &TPid);
            if (TPid != 0) {
                fclose(file);
                return true;
            }
        }
    }

    fclose(file);
    return false;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isDebugToolCmdLine(JNIEnv *env, jobject thiz) {
    char filePath[32], fileRead[128];
    FILE *file;

    snprintf(filePath, 24, "/proc/%d/cmdline", getppid());
    file = fopen(filePath, "r");
    if (file == nullptr) {
        return false;
    }
    fgets(fileRead, 128, file);
    fclose(file);

    if (!strcmp(fileRead, "gdb")) {
        return true;
    }

    if (!strcmp(fileRead, "lldb")) {
        return true;
    }

    return false;
}
extern "C"
JNIEXPORT jint JNICALL
Java_kr_ds_util_CheckDebugNativeLib_version(JNIEnv *env, jobject thiz) {
    return 1;
}