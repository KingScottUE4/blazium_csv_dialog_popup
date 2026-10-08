# Protect Blazium Engine Core
-keep class com.godot.** { *; }
-keep class app.blazium.** { *; }
-keep class org.godotengine.** { *; }
-keep class ** extends app.blazium.godot.plugin.GodotPlugin { *; }

# Protect methods exposed to Blazium via @UsedByGodot
-keepattributes *Annotation*
-keepclassmembers class * {
    @app.blazium.godot.plugin.UsedByGodot *;
}

# Protect JNI bindings
-keepclasseswithmembernames class * {
    native <methods>;
}
-keep public class * extends android.app.Activity
-keep public class * extends android.app.Application
-keep public class * extends android.app.Service
