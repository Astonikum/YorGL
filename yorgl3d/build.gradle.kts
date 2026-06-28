plugins {
    `java-library`
}

group = rootProject.group
version = rootProject.version

java.toolchain.languageVersion.set(JavaLanguageVersion.of(21))

dependencies {
    api(project(":"))
}

val yorgl3dSmoke by tasks.registering(JavaExec::class) {
    group = "verification"
    description = "Runs the lightweight YorGL3D scene smoke check"
    classpath = sourceSets.test.get().runtimeClasspath
    mainClass.set("org.yorgl3d.SceneTest")
    javaLauncher.set(javaToolchains.launcherFor {
        languageVersion.set(JavaLanguageVersion.of(21))
    })
}

tasks.check {
    dependsOn(yorgl3dSmoke)
}
