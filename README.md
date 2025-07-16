# CheckDebugTool
루팅 체크, 디버깅 상태 확인 및 실시간 디버깅 툴 탐지 라이브러리

[![](https://jitpack.io/v/dsoft2010/CheckDebugTool.svg)](https://jitpack.io/#dsoft2010/CheckDebugTool)
---

## 1. 환경 설정

### 1. *루트*  build.gradle 파일에 

[jitpack.io](https://jitpack.io) 로 배포 되어 있어서 

아래와 같이 추가하고

```build.gradle
allprojects {
  repositories {
    // 아래 코드 추가
    maven { url 'https://jitpack.io' }
  }
}
```

혹은 아래와 같이 추가하고
```build.gradle
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
        // 아래 코드 추가
        maven { url 'https://jitpack.io' }
    }
}
```
### 2. 사용자 하고자 하는 *앱*  build.gradle 에 아래와 같이 추가하고
```build.gradle
dependencies {
  // 아래 코드 추가 
  implementation 'com.github.dsoft2010:CheckDebugTool:1.2.3' // 혹은 최신 버전 사용을 위해 latest.release 사용 
}
```

## 2.  사용법
아래와 같이 선언하고
```kotlin
private val checkDebugNativeLib by lazy {
    CheckDebugNativeLib()
}
```

아래와 같이 사용하시면 되고, 샘플 코드 참고하셔도 됩니다.

### 1. 루팅 체크
```kotlin
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
```

### 2. 디버깅 상태 확인 
```kotlin
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
```

### 3. 수동 실시간 디버깅 툴 감지
```kotlin
    override fun onCreate(savedInstanceState: Bundle?) {
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
```

## 3. 변경사항 
- 1.1.0 이상, 1.2.0 미만 버전은 백그라운드로 디버깅 감지 쓰레드가 동작하여 디버깅이 감지되면 앱이 강제 종료됨
- **1.2.0 이상 버전은 백그라운드 디버깅 감지 쓰레드 수동으로 시작/종료하도록 변경됨**
   - startAntiDebugOnBackground 함수로 시작
     - callback 파라미터를 넘기는 callback을 받아서 처리할 수 있다.
     - callback 이 null 일 경우, 기본 동작은 앱 강제 종료이다.
   - stopAntiDebugOnBackground 함수로 종료
- 1.2.1 이상 버전은 targetSDK 35 대응을 위한 16KB 페이지 크기 지원이 추가되었습니다.
- 1.2.2 버전에서는 isRooted() 함수 내부 처리 속도가 좀 개선(pm list package 한번만 수행) 되었습니다. 그리고 ANR 보고가 있어서, 비동기로 사용할 수 있는 isRootedAsync() 함수가 추가되었습니다.
- 1.2.3 버전에서는 isRooted(**context: Context**) 형태로 변경되고, 기존 pm list pacakges 대신 PackageManager 를 통해 루팅 앱 설치 여부를 가져오도록 변경되었습니다. isRootedAsync(**context: Context**) 함수도 PackageManager 사용하기 위해 context 파라미터가 추가 되었습니다.
