# Security Policy

YorGL is an experimental rendering library. It still treats crashes, invalid
native handles, unsafe JNI boundaries, and malformed asset inputs as security
relevant when they can cross a process or trust boundary.

## Supported Versions

Security fixes target the latest `main` branch until the project starts
shipping stable release lines.

## Reporting

Please report security issues privately to the repository owner before opening
a public issue. Include:

- affected commit or tag;
- operating system and graphics backend;
- minimal reproduction steps;
- crash logs, sanitizer output, or debugger notes when available.

Do not include exploit payloads in public issues.
