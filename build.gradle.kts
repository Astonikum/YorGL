plugins {
    kotlin("jvm") version "2.0.21"
    `maven-publish`
}

group = "org.yorgl"
version = providers.gradleProperty("yorglVersion").orElse("0.1.0-SNAPSHOT").get()

java.toolchain.languageVersion.set(JavaLanguageVersion.of(21))

sourceSets.main {
    kotlin.srcDir("bindings/kotlin/src/main/kotlin")
    resources.srcDir(layout.buildDirectory.dir("generated/nativeResources"))
}

val nativeBuildDir = layout.buildDirectory.dir("native")
val nativeResourceDir = layout.buildDirectory.dir("generated/nativeResources/org/yorgl/native/windows-x64")
val isWindows = System.getProperty("os.name").contains("Windows", ignoreCase = true)

val cmakeConfigure by tasks.registering(Exec::class) {
    group = "native"
    description = "Configure YorGL native library"
    commandLine("cmake", "-S", ".", "-B", nativeBuildDir.get().asFile.absolutePath, "-DYORGL_BUILD_DX11=ON", "-DYORGL_BUILD_TESTS=OFF")
    outputs.dir(nativeBuildDir)
}

val cmakeBuild by tasks.registering(Exec::class) {
    group = "native"
    description = "Build YorGL native library"
    dependsOn(cmakeConfigure)
    commandLine("cmake", "--build", nativeBuildDir.get().asFile.absolutePath, "--config", "Release")
    outputs.file(nativeBuildDir.map { it.file("Release/yorgl.dll") })
}

val copyNativeResources by tasks.registering(Copy::class) {
    group = "native"
    description = "Copy native binaries into jar resources on Windows"
    if (isWindows) dependsOn(cmakeBuild)
    onlyIf { isWindows }
    from(nativeBuildDir.map { it.file("Release/yorgl.dll") })
    into(nativeResourceDir)
}

tasks.processResources {
    dependsOn(copyNativeResources)
}

publishing {
    publications {
        create<MavenPublication>("maven") {
            from(components["java"])
            artifactId = "yorgl"
        }
    }
}
