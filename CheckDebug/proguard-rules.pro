# Keep the public API of CheckDebugNativeLib, allowing private members to be obfuscated.
-keep class kr.ds.util.CheckDebugNativeLib {
    public *;
}

# Keep the nested Callback interface, which is part of the public API.
-keep interface kr.ds.util.CheckDebugNativeLib$Callback {
    *;
}

# Keep the names of all native methods (both public and private).
# JNI depends on these exact method names to link from C++ to Kotlin/Java.
-keepclassmembers class kr.ds.util.CheckDebugNativeLib {
    native <methods>;
}

# Keep the Companion object's members, as they are used for static-like access.
-keepclassmembers class kr.ds.util.CheckDebugNativeLib$Companion {
    public *;
}

# Suppress warnings for StringConcatFactory, which is a common issue.
-dontwarn java.lang.invoke.StringConcatFactory
