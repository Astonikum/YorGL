plugins {
    kotlin("jvm") version "2.0.21"
    `maven-publish`
}

group = "org.yorgl"
version = providers.gradleProperty("yorglVersion").orElse("0.1.0-SNAPSHOT").get()

java.toolchain.languageVersion.set(JavaLanguageVersion.of(21))

sourceSets.main {
    kotlin.srcDir("bindings/kotlin/src/main/kotlin")
}

publishing {
    publications {
        create<MavenPublication>("maven") {
            from(components["java"])
            artifactId = "yorgl"
        }
    }
}
