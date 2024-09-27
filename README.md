# CheckDebugTool
루팅 및 디버깅 상태 및 툴 탐지 라이브러리

[![](https://jitpack.io/v/dsoft2010/CheckDebugTool.svg)](https://jitpack.io/#dsoft2010/CheckDebugTool)
---
[jitpack.io](https://jitpack.io) 로 배포 되어 있어서 

1. 루트 build.gradle 파일에 

아래와 같이 추가하고

```
allprojects {
  repositories {
	  ...
    maven { url 'https://jitpack.io' }
  }
}
```

혹은 아래와 같이 추가하고
```
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
        ...
        maven { url 'https://jitpack.io' }
    }
}
```
2. 사용자 하고자 하는 앱 build.gradle 에 아래와 같이 추가하고
```
dependencies {
  ...
  implementation 'com.github.dsoft2010:CheckDebugTool:latest.release'
}
```
3.  사용법은
아래와 같이 선언하고
```
private val checkDebugNativeLib by lazy {
    CheckDebugNativeLib()
}
```

아래와 같이 사용하시면 되고, 샘플 코드 참고하셔도 됩니다.
```
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
        Timber.d(message)
        message
    }
}
```
4. 1.1.0 이상, 1.2.0 미만 버전은 백그라운드로 디버깅 감지 쓰레드가 동작하여 디버깅이 감지되면 앱이 강제 종료됨

5. **1.2.0 이상 버전은 백그라운드 디버깅 감지 쓰레드 수동으로 시작/종료하도록 변경됨**
   - startAntiDebugOnBackground 함수로 시작
     - callback 파라미터를 넘기는 callback을 받아서 처리할 수 있다.
     - callback 이 null 일 경우, 기본 동작은 앱 강제 종료이다.
   - stopAntiDebugOnBackground 함수로 종료
