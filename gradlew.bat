@rem Gradle wrapper script for Windows
@if "%DEBUG%"=="" @echo off
set DIRNAME=%~dp0
set DEFAULT_JVM_OPTS="-Xmx64m" "-Xms64m"
set CLASSPATH=%DIRNAME%\gradle\wrapper\gradle-wrapper.jar
java %DEFAULT_JVM_OPTS% -classpath "%CLASSPATH%" org.gradle.wrapper.GradleWrapperMain %*
