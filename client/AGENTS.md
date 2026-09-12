This project is an extremely security-sensitive chat application designed to resist honeypot-style server compromise and impersonation.

Agents may modify only `.qml` files.

Agents must not modify:

* `.cpp` files
* `.hpp` files
* `CMakeLists.txt`
* build configuration
* backend logic
* networking code
* cryptographic code
* protocol code

The UI should resemble a conventional modern desktop messenger.

Prefer:

* simple layouts
* clear conversation lists
* readable message bubbles
* a message composer at the bottom
* restrained styling
* reusable QML components

Do not implement application logic in QML beyond UI behavior such as layout, animations, navigation, and calling interfaces already exposed by the C++ backend.

Do not invent or modify C++ interfaces. If the UI requires functionality that is not currently exposed to QML, leave the required integration point obvious rather than changing non-QML files.
