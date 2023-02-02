package kr.ds.util

import android.content.Context
import android.os.Debug
import android.provider.Settings
import java.io.File

class CheckDebugNativeLib {

    /**
     * 루팅 여부 체크
     */
    fun isRooted(): Boolean {
        val isResult: Boolean
        val isRooting: Boolean = try {
            Runtime.getRuntime().exec("su")
            true
        } catch (e: java.lang.Exception) {
            Debug.isDebuggerConnected()
        }
        var isRooting2 = false
        val paths = arrayOf(
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
            "/su/bin/su"
        )
        for (p in paths) {
            if (File(p).exists()) {
                isRooting2 = true
            }
        }
        isResult = isRooting2 || isRooting
        return isResult
    }

    /**
     * 디버깅 툴 활성화 여부
     */
    fun isActiveDebugTool(): Boolean {
        return isDebugToolCmdLine() || isDebugToolTracerPid()
    }

    /**
     * 개발자 모드 활성화 여부
     */
    fun isDevelopmentSettingsEnabled(context: Context) =
        Settings.Global.getInt(context.contentResolver, Settings.Global.DEVELOPMENT_SETTINGS_ENABLED) != 0

    /**
     * USB Debugging 활성화 여부
     */
    fun isUsbDebuggingEnabled(context: Context) =
        Settings.Global.getInt(context.contentResolver, Settings.Global.ADB_ENABLED) != 0

    /**
     * Native 라이브러리 버전
     *
     */
    external fun version(): Int

    /**
     * 디버깅 툴에 의해 디버그 실행 중인 여부
     */
    private external fun isDebugToolTracerPid(): Boolean

    /**
     * 디버깅 툴 cmdline 여부
     */
    private external fun isDebugToolCmdLine(): Boolean

    companion object {
        // Used to load the 'check_debug_util' library on application startup.
        init {
            System.loadLibrary("check_debug_util")
        }
    }
}