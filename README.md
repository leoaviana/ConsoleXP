# ConsoleXP

**ConsoleXP** is a modern enhancement mod for **World of Warcraft 3.3.5a (WotLK)** that brings several quality-of-life features from retail WoW to the old client. It consists of a DLL to be injected into the game and a matching AddOn to configure and interact with these features in-game.

> ⚠️ **This mod is guaranteed to work in local play. Compatibility with private servers is not guaranteed, although it may work.
Custom servers that heavily modify the client — such as Ascension — might not be compatible.**

---

## 🎮 Features

* 🎥 **Dynamic Camera**
  Emulates the [ActionCam](https://wowpedia.fandom.com/wiki/CVar_ActionCam) system introduced in Legion. Features include:

  * Head bobbing
  * Target tracking (pitch and yaw)
  * Custom camera pitch, height, and over-the-shoulder offsets

  You use this feature with full customization consider using my port of [DynamicCam here.](https://github.com/leoaviana/DynamicCamLK)

* 🎯 **Action Targeting**
  Enables targeting enemies by aiming at them—just like in retail WoW (Dragonflight). It dynamically targets the mob based on your view direction, and works alongside the standard targeting system.

* 🔁️ **Interact Button**
  Press the Interact Key to interact with NPCs and objects using a keypress instead of your mouse. Interactive elements will highlight as you enter range.

* 🔁️ **Interact on MouseOver**
  Press the Interact On MouseOver key to interact with NPCs and objects that are currently under the virtual mouse created by ConsoleXP.

* 📡 **Improved Targeting System**
  ported from the **UnitXP\_SP3** vanilla mod for smarter and more accurate target acquisition. Includes multiple targeting modes:

  * Target Nearest Enemy
  * Raid Mark Targeting
  * Melee Range Targeting
  * Ranged Targeting
  * World Boss Targeting

* 🧹 **ConsoleXP AddOn**
  Provides an interface for configuring features. it is **highly recommended**, especially for enabling ActionTarget UI frames, crosshair and other small settings.

---

## 📦 Release Structure

* `ConsoleXP.dll`: Injected into the game to enable features.
* `ConsoleXPPatcher.exe`: Use this to patch your `Wow.exe` so it can load ConsoleXP.dll on startup
* `ConsoleXP/`: AddOn folder to be placed inside `Interface/AddOns`.

---

## 🧹 Patcher Information

You must use the patcher contained in the release to patch your client to load ConsoleXP.dll. It also implements the ability to load AwesomeWotlkLib.dll if you intend to use it. I believe it will work fine, I tested with the [original](https://github.com/FrostAtom/awesome_wotlk) version, not the [forked one](https://github.com/someweirdhuman/awesome_wotlk) but I believe it will work too.

---

## ❌ Disclaimer

* **Warden**: Use at your own risk, some servers—especially custom ones like **Ascension**—may use anti-cheat systems or patched clients that crash, block, or ban modified function hooks, you've been warned.
* **False positives in antivirus**: Injectors and patchers are often flagged as malware by AV software.
* **Not a cheat**: ConsoleXP does not automate gameplay or give unfair advantages. It adds only quality-of-life improvements and control enhancements.

---


## 🛠️ Build It Yourself

If you're concerned about binaries that might've been flagged by your antivirus, you are free to build the sources yourself:

```bash
git clone https://github.com/your/ConsoleXP.git
cd ConsoleXP
# Open the solution with Visual Studio
```

---

## 🙏 Acknowledgements

Special thanks to the authors of the following projects:

* [CppBot](https://github.com/mtm88/CppBot): Great object manager reference.
* [awesome\_wotlk](https://github.com/FrostAtom/awesome_wotlk): Excellent source of reversed game functions and interfaces. Also used their client patcher.
* [UnitXP\_SP3](https://github.com/allfoxwy/UnitXP_SP3): For smart targeting logic.
* [IceFlake](https://github.com/miceiken/IceFlake): Used early on for ActionCam logic in C#.
* [Interact](https://github.com/luskanek/Interact): Base reference for interact feature.

---

## 📜 License

MIT License. Use freely. Attribution is appreciated but **not required**.

If a project that modifies the WoW client wishes to integrate ConsoleXP, they are welcome to do so under the terms of the MIT license.
