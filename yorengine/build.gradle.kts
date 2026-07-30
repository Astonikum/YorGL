plugins {
    `java-library`
}

group = rootProject.group
version = rootProject.version

java.toolchain.languageVersion.set(JavaLanguageVersion.of(21))

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
}

tasks.check {
    dependsOn(yorengineSmoke)
}
