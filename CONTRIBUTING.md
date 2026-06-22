# Contributing

- Keep public API documented in `docs/`.
- One backend feature must work in at least one backend before it enters the API.
- Prefer deleting duplicate backend code over adding helper layers.
- Keep client/game-specific code outside YorGL.
- Update Kotlin bindings when the C ABI changes.
- Run `./gradlew build` and native smoke tests before opening a PR.

Branches:

- `main` - stable line.
- `<version>` - version line.
- `<version>-<task>` - task branch.
