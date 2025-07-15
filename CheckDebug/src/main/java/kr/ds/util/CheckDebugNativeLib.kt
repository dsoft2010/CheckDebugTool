package kr.ds.util

import android.content.Context
import android.content.pm.ApplicationInfo
import android.os.Build
import android.os.Debug
import android.provider.Settings
import android.util.Log
import androidx.annotation.Keep


class CheckDebugNativeLib {

    /**
     * 루팅 여부 체크
     */
    fun isRooted(): Boolean {
        val rootingByFileExistence = isRootingByFileExistence()
        val rootingByExecCmd = isRootingByExecCmd()
        Log.d("CheckDebug", "isRootingByFileExistence: $rootingByFileExistence, isRootingByExecCmd: $rootingByExecCmd")
        return rootingByFileExistence || rootingByExecCmd || isTestKeyBuild()
    }

    /**
     * 디버깅 툴 활성화 여부
     *
     * 디버거 연결, CMD 라인, TracerPid, Timber Check 로 확인
     */
    fun isActiveDebugTool(): Boolean {
        return isDebuggerConnected()  || isDebugToolCmdLine() || isDebugToolTracerPid() || (!BuildConfig.DEBUG && isDebugByTimerChecks())
    }

    /**
     * 개발자 모드 활성화 여부
     */
    fun isDevelopmentSettingsEnabled(context: Context): Boolean {
        return try {
            val isEnabled = Settings.Global.getInt(
                context.contentResolver,
                Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0
            ) != 0
            Log.d("CheckDebug", "isDevelopmentSettingsEnabled: $isEnabled")
            isEnabled
        } catch (e: Settings.SettingNotFoundException) {
            Log.e("CheckDebug", e.localizedMessage, e)
            false
        }
    }

    /**
     * USB Debugging 활성화 여부
     */
    fun isUsbDebuggingEnabled(context: Context) =
        Settings.Global.getInt(context.contentResolver, Settings.Global.ADB_ENABLED) != 0

    /**
     * Debugging 활성화 여부
     *
     * android:debuggable or ro.debuggable
     */
    fun isDebuggable(context: Context): Boolean {
        val flagDebuggable =
            context.applicationContext.applicationInfo.flags and ApplicationInfo.FLAG_DEBUGGABLE != 0
        Log.d("CheckDebug", "flagDebuggable: $flagDebuggable")
        return flagDebuggable || isDebugEnabled()
    }

    /**
     * Debugger 연결 여부
     */
    private fun isDebuggerConnected(): Boolean {
        val debuggerConnected = Debug.isDebuggerConnected()
        Log.d("CheckDebug", "isDebuggerConnected: $debuggerConnected")
        return debuggerConnected
    }

    /**
     * Debugging 여부 by Timer Checks
     */
    private fun isDebugByTimerChecks(): Boolean {
        val start = Debug.threadCpuTimeNanos()
        for (i in 0..1000000)
            continue
        val stop = Debug.threadCpuTimeNanos()

        val result = stop - start >= 10000000
        Log.d("CheckDebug", "isDebugByTimerChecks: $result")
        return result
    }

    private fun isTestKeyBuild(): Boolean {
        return Build.TAGS?.contains("test-keys") ?: false
    }

    /**
     * Native 라이브러리 버전
     *
     */
    external fun version(): Int

    /**
     * 루팅 여부 체크 by File Existence
     */
    private external fun isRootingByFileExistence(): Boolean

    /**
     * 루팅 여부 체크 by Exec Cmd
     */
    private external fun isRootingByExecCmd(): Boolean

    /**
     * 루팅 여부 체크 by File Existence 비동기
     */
    private external fun isRootingByFileExistenceAsync(callback: (Boolean) -> Unit)

    /**
     * 루팅 여부 체크 비동기
     */
    fun isRootedAsync(callback: (Boolean) -> Unit) {
        isRootingByFileExistenceAsync { isRootedByFile ->
            val isRootedByExec = isRootingByExecCmd()
            val isTestKey = isTestKeyBuild()
            val result = isRootedByFile || isRootedByExec || isTestKey
            Log.d("CheckDebug", "isRootedAsync: $result, isRootedByFile: $isRootedByFile, isRootedByExec: $isRootedByExec, isTestKey: $isTestKey")
            callback(result)
        }
    }

    /***
     * Debugging 활성화 여부
     *
     * ro.debuggable
     */
    private external fun isDebugEnabled(): Boolean

    /**
     * 디버깅 툴에 의해 디버그 실행 중인 여부
     */
    private external fun isDebugToolTracerPid(): Boolean

    /**
     * 디버깅 툴 cmdline 여부
     */
    private external fun isDebugToolCmdLine(): Boolean

    /**
     * ptrace 선점을 통한 디버거 process attach 방지
     *
     * ** 테스트 중 **
     *
     * 이 함수를 실행 한 후에는 isDebugToolTracerPid() 가 항상 true 이다.
     *
     * @see <a href=https://mas.owasp.org/MASTG/0x05j-Testing-Resiliency-Against-Reverse-Engineering/>Android Anti-Reversing Defenses</a>
     */
    private external fun antiDebug()


    /**
     * 디버깅 툴에 의해 디버그 실행 중인지 확인하는 콜백
     */
    @Keep
    interface Callback {
        fun onDebug()
    }

    companion object {

        external fun startAntiDebugOnBackground(callback: Callback? = null)
        external fun stopAntiDebugOnBackground()

        // Used to load the 'check_debug_util' library on application startup.
        init {
            System.loadLibrary("check_debug_util")
        }
    }
}