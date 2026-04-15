# UE Mobile Dumper

UE Mobile Dumper is a lightweight tool for dumping Unreal Engine (UE4/UE5) mobile games on Android.

Unlike traditional dumpers, this tool **does NOT require root access**. It can be **directly injected into the APK** and accessed through an **in-game floating window UI**.

---

## ✨ Features

- 🚫 No root required
- 📦 Works via APK injection
- 🪟 Built-in floating window interface (in-game overlay)
- 🧠 Dump core UE structures:
    - GNames / FNamePool
    - GUObjectArray
    - GWorld
- 🧾 SDK generation (classes, structs, enums, functions)
- 📚 Dump strings and objects
- ⚡ Fast dumping mode
- 🛠 Supports 32-bit & 64-bit UE4/UE5 games
- 🔧 Dump and rebuild `libUE4.so`

---

## ⚙️ Requirements

- Android device (No root needed)
- APK editor / injector tool
- Android NDK (for building)
- Basic knowledge of UE offsets

---

## 🛠️ Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/AscarreX/UE4-Mobile-Dumper.git
cd UE4-Mobile-Dumper
```

2. Build using Android Studio

3. Output will be generated in `app/src/main/jni/libs`

4. Copy the libs and Dex generated and put in target Game

5. Make sure to put the Main Activity start code to actually start the menu.

Note - You only need to run the start and no need to load the library seperately

---

## 🚀 Usage (Injection Method)

1. Inject the compiled `.so` into the target APK
2. Load the library (JNI or manual injection)
3. Launch the game
4. Open the **floating window overlay**
5. Use the UI to:
    - Dump SDK
    - Dump strings
    - Dump objects
    - Dump `libUE4.so`

---

## 🧾 Features via UI

- SDK Dump (GUObject / GWorld based)
- String Dump
- Object Dump
- Memory Dump

All actions are accessible directly from the in-game overlay.

---

## ⚠️ Notes

- Offsets must be obtained manually for each game
- Modified or protected UE builds may require adjustments
- Some games may block overlays or injections
- Use correct architecture (armv7 / arm64)

---

## 🐞 Troubleshooting

| Issue | Solution |
|------|---------|
| No data dumped | Check offsets |
| Crash on injection | Wrong architecture or bad hook |
| Overlay not visible | Game blocking draw-over apps |
| Invalid SDK | Incorrect GNames / GUObject |

---

## 📌 TODO

- [ ] Auto offset finder (pattern scanning)
- [ ] Better support for protected builds
- [ ] One-click injection tool

---

## ⚖️ License

Licensed under the GNU AGPL License.

---

## ⚠️ Disclaimer

This project is for **educational and research purposes only**.

Do NOT use it for:
- Cheating in online games
- Bypassing protections
- Any illegal activity

You are responsible for your own usage.

---

## 🙌 Credits

- UE4 reverse engineering community
- Inspired by various UE dumping tools
- Contributors
- kp7742
- maiyao1988

---

## ⭐ Support

If you find this project useful:

- Star the repo ⭐
- Share it
- Contribute improvements  