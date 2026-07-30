plugins {
    `java-library`
}

group = rootProject.group
version = rootProject.version

java.toolchain.languageVersion.set(JavaLanguageVersion.of(21))

val isWindows = System.getProperty("os.name").contains("Windows", ignoreCase = true)
val nativeResourceRootDir = layout.buildDirectory.dir("generated/nativeResources")
val nativeResourceDir = layout.buildDirectory.dir("generated/nativeResources/org/yorengine/native/windows-x64")
val copyNativeResources by tasks.registering(Copy::class) {
    group = "native"
    description = "Copy the native YorEngine binding into the JVM artifact"
    if (isWindows) dependsOn(rootProject.tasks.named("cmakeBuild"))
    onlyIf { isWindows }
    from(rootProject.layout.buildDirectory.dir("native/yorengine/Release")) {
        include("yorengine_api.dll")
    }
    into(nativeResourceDir)
}

sourceSets.main {
    resources.srcDir(nativeResourceRootDir)
}

tasks.processResources {
    dependsOn(copyNativeResources)
}

dependencies {
    api(project(":"))
}

val yorengineSmoke by tasks.registering(JavaExec::class) {
    group = "verification"
    description = "Runs the YorEngine scene smoke check"
    classpath = sourceSets.test.get().runtimeClasspath
    mainClass.set("org.yorengine.SceneTest")
    javaLauncher.set(javaToolchains.launcherFor {
        languageVersion.set(JavaLanguageVersion.of(21))
    })
    dependsOn(tasks.named("testClasses"))
}

val yorengineNativeSmoke by tasks.registering(JavaExec::class) {
    group = "verification"
    description = "Runs the Windows JVM adapter smoke check against C++ YorEngine"
    classpath = sourceSets.test.get().runtimeClasspath
    mainClass.set("org.yorengine.NativeSceneTest")
    javaLauncher.set(javaToolchains.launcherFor {
        languageVersion.set(JavaLanguageVersion.of(21))
    })
    dependsOn(tasks.named("testClasses"), copyNativeResources)
    onlyIf { isWindows }
}

tasks.check {
    dependsOn(yorengineSmoke, yorengineNativeSmoke)
}
