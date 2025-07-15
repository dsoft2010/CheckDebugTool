package com.example.checkdebugtoolsample

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.example.checkdebugtoolsample.databinding.ActivityMainBinding
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

    private lateinit var binding: ActivityMainBinding
    private val checkDebugNativeLib by lazy {
        CheckDebugNativeLib()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        Timber.d("CheckDebugTool version=${checkDebugNativeLib.version()}")

        // Async Rooting Check
        binding.rootingStatusAsync.text = "Async Rooting Status: Checking..."
        checkDebugNativeLib.isRootedAsync(this) { isRooted ->
            val message = if (isRooted) {
                "Async Rooting Status: Rooted"
            } else {
                "Async Rooting Status: Not Rooted"
            }
            Timber.d(message)
            binding.rootingStatusAsync.text = message
        }

        // Sync Rooting Check
        val isRootedSync = checkDebugNativeLib.isRooted(this)
        val syncRootingMessage = if (isRootedSync) {
            "Sync Rooting Status: Rooted"
        } else {
            "Sync Rooting Status: Not Rooted"
        }
        Timber.d(syncRootingMessage)
        binding.rootingStatusSync.text = syncRootingMessage

        // Debug Tool Check
        val isActiveDebugTool = checkDebugNativeLib.isActiveDebugTool()
        val debugToolMessage = if (isActiveDebugTool) {
            "Debug Tool Status: Detected"
        } else {
            "Debug Tool Status: Not Detected"
        }
        Timber.d(debugToolMessage)
        binding.debugToolStatus.text = debugToolMessage

        // Debuggable Check
        if (!BuildConfig.DEBUG) {
            val isDebuggable = checkDebugNativeLib.isDebuggable(this)
            val debuggableMessage = if (isDebuggable) {
                "Debuggable Status: Enabled"
            } else {
                "Debuggable Status: Disabled"
            }
            Timber.d(debuggableMessage)
            binding.debuggableStatus.text = debuggableMessage
        } else {
            binding.debuggableStatus.text = "Debuggable Status: (Debug Build)"
        }

        // USB Debugging Check
        if (!BuildConfig.DEBUG) {
            val isUsbDebuggingEnabled = checkDebugNativeLib.isUsbDebuggingEnabled(this)
            val usbDebuggingMessage = if (isUsbDebuggingEnabled) {
                "USB Debugging Status: Enabled"
            } else {
                "USB Debugging Status: Disabled"
            }
            Timber.d(usbDebuggingMessage)
            binding.usbDebuggingStatus.text = usbDebuggingMessage
        } else {
            binding.usbDebuggingStatus.text = "USB Debugging Status: (Debug Build)"
        }

        // Development Mode Check
        if (!BuildConfig.DEBUG) {
            val isDevelopmentSettingsEnabled = checkDebugNativeLib.isDevelopmentSettingsEnabled(this)
            val developmentModeMessage = if (isDevelopmentSettingsEnabled) {
                "Development Mode Status: Enabled"
            } else {
                "Development Mode Status: Disabled"
            }
            Timber.d(developmentModeMessage)
            binding.developmentModeStatus.text = developmentModeMessage
        } else {
            binding.developmentModeStatus.text = "Development Mode Status: (Debug Build)"
        }

        // Anti-Debug
        if (!isRootedSync && !isActiveDebugTool) {
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
            Timber.d("Anti-debugging background process started.")
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        CheckDebugNativeLib.stopAntiDebugOnBackground()
    }
}
