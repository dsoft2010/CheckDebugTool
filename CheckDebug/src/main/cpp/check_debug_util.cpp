#include <jni.h>
#include <string>
#include <unistd.h>
#include <android/log.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <fstream>

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

bool fileExists(const std::string &filePath) {
    struct stat info;
    return ( stat( filePath.c_str(), &info ) == 0 );
}

bool checkRootPath(const std::string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR) != 0;
}

std::string findExecutable(const std::string &command) {
    std::string path;
    std::string cmd = "which " + command + " 2>/dev/null";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            path = buffer;
            // Remove trailing newline character
            if (!path.empty() && path.back() == '\n') {
                path.pop_back();
            }
        }
        pclose(pipe);
    }

    return path;
}

static bool isRooted() {
    // Check for common rooted paths
    std::vector<std::string> rootedPaths = {
            "/sbin/su",
            "/system/su",
            "/system/bin/su",
            "/system/sbin/su",
            "/system/xbin/su",
            "/system/xbin/mu",
            "/system/bin/.ext/.su",
            "/system/usr/su-backup",
            "/data/data/com.noshufou.android.su",
            "/system/app/Superuser.apk",
            "/system/app/su.apk",
            "/system/bin/.ext",
            "/system/xbin/.ext",
            "/data/local/xbin/su",
            "/data/local/bin/su",
            "/system/sd/xbin/su",
            "/system/bin/failsafe/su",
            "/data/local/su",
            "/su/bin/su",
            "/sbin/magisk",
    };

    for (const std::string &path: rootedPaths) {
        if (fileExists(path) || checkRootPath(path)) {
            LOGD("rooted path: %s", (path.c_str()));
            return true;
        }
    }

    return false;
}

bool isCheckExecution() {
    std::string suPath = findExecutable("su");
    if (!suPath.empty()) {
        return true;
    }

    return false;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isDebugToolTracerPid(JNIEnv *, jobject) {
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
Java_kr_ds_util_CheckDebugNativeLib_isDebugToolCmdLine(JNIEnv *, jobject) {
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

const int version = 3;

extern "C"
JNIEXPORT jint JNICALL
Java_kr_ds_util_CheckDebugNativeLib_version(JNIEnv *env, jobject) {
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
        pthread_create(&t, nullptr, reinterpret_cast<void *(*)(void *)>(monitor_pid),
                       (void *) nullptr);
    }
}

static pthread_key_t g_key;
static JavaVM *pJavaVm;
static jobject callback_object = nullptr;

static JNIEnv *getEnv();

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
                        fclose(fd);

                        if (callback_object) {
                            JNIEnv *_env = getEnv();
                            jclass clazz = _env->GetObjectClass(callback_object);
                            jmethodID method = _env->GetMethodID(clazz, "onDebug", "()V");
                            _env->CallVoidMethod(callback_object, method);
                        } else {
                            LOGD("be attached !! kill %d", pid);
                            int ret = kill(pid, SIGKILL);
                        }


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

static bool isAliveAntiDebugByProcessStatus = false;

void thread_task(int n) {

    while (isAliveAntiDebugByProcessStatus) {

        LOGV("start be_attached_check...");

        be_attached_check();

        std::this_thread::sleep_for(std::chrono::seconds(n));

    }

}

void anti_debug_by_process_status() {

    LOGD("called anti_debug_by_process_status");

    auto checkThread = std::thread(thread_task, 1);

    checkThread.detach();

}

JavaVM *getJavaVM() {
    pthread_t thisThread = pthread_self();
    LOGD("getJavaVM(), pthread_self() = %ld", thisThread);
    return pJavaVm;
}

static void detachCurrentThread(void *a) {
    getJavaVM()->DetachCurrentThread();
}

void setJavaVM(JavaVM *javaVM) {
    pthread_t thisThread = pthread_self();
    LOGD("setJavaVM(%p), pthread_self() = %ld", javaVM, thisThread);
    pJavaVm = javaVM;

    pthread_key_create(&g_key, detachCurrentThread);
}

JNIEnv *cacheEnv(JavaVM *jvm) {
    JNIEnv *_env = nullptr;
    // get jni environment
    jint ret = jvm->GetEnv((void **) &_env, JNI_VERSION_1_6);

    switch (ret) {
        case JNI_OK :
            // Success!
            pthread_setspecific(g_key, _env);
            return _env;

        case JNI_EDETACHED :
            // Thread not attached
            if (jvm->AttachCurrentThread(&_env, nullptr) < 0) {
                LOGE("Failed to get the environment using AttachCurrentThread()");

                return nullptr;
            } else {
                // Success : Attached and obtained JNIEnv!
                pthread_setspecific(g_key, _env);
                return _env;
            }

        case JNI_EVERSION :
            // Cannot recover from this error
            LOGE("JNI interface version 1.6 not supported");
        default :
            LOGE("Failed to get the environment using GetEnv()");
            return nullptr;
    }
}

JNIEnv *getEnv() {
    auto *_env = (JNIEnv *) pthread_getspecific(g_key);
    if (_env == nullptr)
        _env = cacheEnv(pJavaVm);
    return _env;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;

    setJavaVM(vm);

    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {

        return -1;

    }

    return JNI_VERSION_1_6;

}

extern "C"
JNIEXPORT void JNICALL
Java_kr_ds_util_CheckDebugNativeLib_antiDebug(JNIEnv *env, jobject) {
    anti_debug_by_preemption_ptrace();
}



extern "C"
JNIEXPORT void JNICALL
Java_kr_ds_util_CheckDebugNativeLib_00024Companion_startAntiDebugOnBackground(JNIEnv *env,
                                                                              jobject,
                                                                              jobject callback) {
    if (isAliveAntiDebugByProcessStatus) {
        return;
    }
    LOGD("startAntiDebugOnBackground %p", callback);
    callback_object = env->NewGlobalRef(callback);

    isAliveAntiDebugByProcessStatus = true;
    anti_debug_by_process_status();
}

extern "C"
JNIEXPORT void JNICALL
Java_kr_ds_util_CheckDebugNativeLib_00024Companion_stopAntiDebugOnBackground(JNIEnv *env,
                                                                             jobject) {
    isAliveAntiDebugByProcessStatus = false;
    if (callback_object) {
        env->DeleteGlobalRef(callback_object);
        callback_object = nullptr;
    }
}

static bool isDebuggable() {
    std::string getpropCmd = "getprop ro.debuggable";
    std::string debuggableValue;
    FILE *pipe = popen(getpropCmd.c_str(), "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            debuggableValue = buffer;

            if (!debuggableValue.empty() && debuggableValue.back() == '\n') {
                debuggableValue.pop_back();
            }
        }
        pclose(pipe);
    }

    LOGD("ro.debuggable: %s", debuggableValue.c_str());
    return (debuggableValue == "1");
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isDebugEnabled(JNIEnv *env, jobject) {
    return isDebuggable() ? JNI_TRUE : JNI_FALSE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isRootingByFileExistence(JNIEnv *env, jobject) {
    return isRooted() ? JNI_TRUE : JNI_FALSE;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isRootingByExecCmd(JNIEnv *env, jobject) {
    return isCheckExecution() ? JNI_TRUE : JNI_FALSE;
}

struct CallbackData {
    JavaVM *vm;
    jobject callback;
};

void* rootingCheckThread(void* arg) {
    CallbackData* data = static_cast<CallbackData*>(arg);
    JavaVM* vm = data->vm;
    jobject callback = data->callback;

    JNIEnv* env;
    vm->AttachCurrentThread(&env, nullptr);

    bool rooted = isRooted();

    jclass callbackClass = env->GetObjectClass(callback);
    // For a Kotlin lambda (Boolean) -> Unit, the method is invoke(Ljava/lang/Object;)Ljava/lang/Object;
    jmethodID invokeMethod = env->GetMethodID(callbackClass, "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");

    if (invokeMethod) {
        // Create a java.lang.Boolean object from the C++ bool
        jclass booleanClass = env->FindClass("java/lang/Boolean");
        jmethodID booleanConstructor = env->GetMethodID(booleanClass, "<init>", "(Z)V");
        jobject booleanArg = env->NewObject(booleanClass, booleanConstructor, rooted);

        // Call the invoke method
        env->CallObjectMethod(callback, invokeMethod, booleanArg);

        env->DeleteLocalRef(booleanArg);
        env->DeleteLocalRef(booleanClass);
    } else {
        // Log an error if the method isn't found
        LOGE("Could not find 'invoke' method on callback object");
    }
    env->DeleteLocalRef(callbackClass);

    env->DeleteGlobalRef(callback);
    delete data;

    vm->DetachCurrentThread();
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_kr_ds_util_CheckDebugNativeLib_isRootingByFileExistenceAsync(JNIEnv *env, jobject, jobject callback) {
    pthread_t thread;
    CallbackData* data = new CallbackData();
    env->GetJavaVM(&data->vm);
    data->callback = env->NewGlobalRef(callback);

    pthread_create(&thread, nullptr, rootingCheckThread, data);
    pthread_detach(thread);
}