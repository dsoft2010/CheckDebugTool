#include <jni.h>
#include <string>
#include <unistd.h>
#include <android/log.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <pthread.h>
#include <chrono>
#include <thread>

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
                LOGD("isDebugToolTracerPid TracerPid: %d true", TPid);
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

const int version = 2;

extern "C"
JNIEXPORT jint JNICALL
Java_kr_ds_util_CheckDebugNativeLib_version(JNIEnv *env, jobject thiz) {
    LOGD("version = %d", version);
    return version;
}

static int child_pid;

void *monitor_pid() {

    int status;

    LOGD("monitor_pid child_pid: %d, enter", child_pid);
    waitpid(child_pid, &status, 0);

    /* Child status should never change. */

    LOGD("monitor_pid exit(0)");
    _exit(0); // Commit seppuku

}

void anti_debug_by_preemption_ptrace() {

    child_pid = fork();

    LOGD("anti_debug_by_preemption_ptrace child_pid: %d", child_pid);

    if (child_pid == 0) {
        int ppid = getppid();
        int status;

        LOGD("anti_debug_by_preemption_ptrace child_pid == 0 enter");

        if (ptrace(PTRACE_ATTACH, ppid, NULL, NULL) == 0) {
            LOGD("anti_debug_by_preemption_ptrace PTRACE_ATTACH ptrace -> 0 enter");
            waitpid(ppid, &status, 0);

            ptrace(PTRACE_CONT, ppid, NULL, NULL);
            LOGD("anti_debug_by_preemption_ptrace PTRACE_CONT");

            while (waitpid(ppid, &status, 0)) {

                LOGD("anti_debug_by_preemption_ptrace waitpid is true");
                if (WIFSTOPPED(status)) {
                    ptrace(PTRACE_CONT, ppid, NULL, NULL);
                    LOGD("anti_debug_by_preemption_ptrace PTRACE_CONT in while");
                } else {
                    // Process has exited
                    LOGD("anti_debug_by_preemption_ptrace exit(0)");
                    _exit(0);
                }
            }
        } else {
            LOGD("anti_debug_by_preemption_ptrace child_pid != 0 leave");
        }

    } else {
        pthread_t t;

        /* Start the monitoring thread */
        LOGD("anti_debug_by_preemption_ptrace Start the monitoring thread");
        pthread_create(&t, NULL, reinterpret_cast<void *(*)(void *)>(monitor_pid), (void *) NULL);
    }
}

void be_attached_check() {

    try {

        const int bufsize = 1024;

        char filename[bufsize];

        char line[bufsize];

        int pid = getpid();

        sprintf(filename, "/proc/%d/status", pid);

        FILE *fd = fopen(filename, "r");

        if (fd != nullptr) {

            while (fgets(line, bufsize, fd)) {

                if (strncmp(line, "TracerPid", 9) == 0) {

                    int statue = atoi(&line[10]);

                    LOGV("%s", line);

                    if (statue != 0) {

                        LOGD("be attached !! kill %d", pid);

                        fclose(fd);

                        int ret = kill(pid, SIGKILL);

                    }

                    break;

                }

            }

            fclose(fd);

        } else {

            LOGD("open %s fail...", filename);

        }

    } catch (...) {
    }
}

void thread_task(int n) {

    while (true) {

        LOGV("start be_attached_check...");

        be_attached_check();

        std::this_thread::sleep_for(std::chrono::seconds(n));

    }

}

void anti_debug_by_process_status() {

    LOGD("call anti_debug_by_preemption_ptrace by process status ......");

    auto checkThread = std::thread(thread_task, 1);

    checkThread.detach();

}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    anti_debug_by_process_status();

    JNIEnv *env;

    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {

        return -1;

    }

    return JNI_VERSION_1_6;

}

extern "C"
JNIEXPORT void JNICALL
Java_kr_ds_util_CheckDebugNativeLib_antiDebug(JNIEnv *env, jobject thiz) {
    anti_debug_by_preemption_ptrace();
}