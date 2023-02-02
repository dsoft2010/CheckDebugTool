package com.example.checkdebugtoolsample

import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import kr.ds.util.BuildConfig
import kr.ds.util.CheckDebugNativeLib
import timber.log.Timber

class MainActivity : AppCompatActivity() {

    private val checkDebugNativeLib by lazy {
        CheckDebugNativeLib()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val textState: TextView = findViewById(R.id.textState)

        textState.text = when {
            !BuildConfig.DEBUG && checkDebugNativeLib.isRooted() -> {
                val message = "루팅됨"
                Timber.d(message)
                message
            }
            !BuildConfig.DEBUG && checkDebugNativeLib.isActiveDebugTool() -> {
                val message = "디버깅 툴(커맨드 라인 or 프로세스) 감지됨"
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
                Timber.d(message)
                message
            }
        }
    }
}