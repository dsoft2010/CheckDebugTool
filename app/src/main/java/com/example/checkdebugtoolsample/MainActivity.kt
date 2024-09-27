package com.example.checkdebugtoolsample

import android.os.Bundle
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kr.ds.util.BuildConfig
import kr.ds.util.CheckDebugNativeLib
import timber.log.Timber
import java.lang.ref.WeakReference
import kotlin.system.exitProcess

class MainActivity : AppCompatActivity() {

    private val checkDebugNativeLib by lazy {
        CheckDebugNativeLib()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        Timber.d("CheckDebugTool version=${checkDebugNativeLib.version()}")

        val textState: TextView = findViewById(R.id.textState)

        textState.text = when {
            checkDebugNativeLib.isRooted() -> {
                val message = "루팅됨"
                Timber.d(message)
                message
            }

            checkDebugNativeLib.isActiveDebugTool() -> {
                val message = "디버깅 툴(커맨드 라인 or 프로세스 or 디버거 연결 or (Timer Checks)) 감지됨"
                Timber.d(message)
                message
            }

            !BuildConfig.DEBUG && checkDebugNativeLib.isDebuggable(this) -> {
                val message = "디버깅 활성화 감지됨"
                Timber.d(message)
                message
            }

            !BuildConfig.DEBUG && checkDebugNativeLib.isUsbDebuggingEnabled(this) -> {
                val message = "USB 디버깅 활성화 감지됨"
                Timber.d(message)
                message
            }

            !BuildConfig.DEBUG && checkDebugNativeLib.isDevelopmentSettingsEnabled(this) -> {
                val message = "개발자 모드 활성화 감지됨"
                Timber.d(message)
                message
            }

            else -> {
                val message = "정상"
                val thisActivity = WeakReference(this@MainActivity)
                val callback = object : CheckDebugNativeLib.Callback {
                    override fun onDebug() {
                        val msg = "!!! Anti Debug catch Debugging !!!"
                        Timber.e(msg)

                        CoroutineScope(Dispatchers.Main).launch {
                            thisActivity.get()?.let { activity ->
                                Toast.makeText(
                                    activity,
                                    "디버깅 툴(프로세스)이 감지되어 앱을 강제 종료합니다.",
                                    Toast.LENGTH_LONG
                                ).show()
                                activity.finish()
                                delay(100)
                                exitProcess(0)
                            }
                        }
                    }

                }
                CheckDebugNativeLib.startAntiDebugOnBackground(callback)
                Timber.d(message)
                message
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        CheckDebugNativeLib.stopAntiDebugOnBackground()
    }
}