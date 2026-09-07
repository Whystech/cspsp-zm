# CSPSP:ZM

![screenshot1]
![screenshot2]

CSPSP:ZM is a homebrew game mod for the Sony PSP based on CSPSP, created in 2026.
It is a 2d top-down shooter mod based on Kevinbchen's CSPSP. The mod only works offline. 
It has 2 game modes: Infection and Extermination.

This mod is based on Kevinbchen's CSPSP which can be reviewed below.
The game is still available to download - see the [Releases](https://github.com/kevinbchen/cspsp/releases) page.

> **Warning**: A lot of refactoring has been carried throughout, I have tried to comment some of the original code so it is easier to find and edit.

## Playing

Simply copy the game into ms0:/PSP/GAME/ like any other homebrew. 
CSPSP ZM is based on the v1.92b release.

![controls](https://user-images.githubusercontent.com/2881968/173964080-ab480a79-af94-454a-bdfe-a2a56b491cad.png)

### PPSSPP
You can also run CSPSP using the [PPSSPP](https://www.ppsspp.org/) emulator, though the latest official release does not yet support online functionality. However, online/infrastructure support is in development (https://github.com/hrydgard/ppsspp/issues/14256), and you can find links to test builds that do already support CSPSP online (e.g. https://github.com/hrydgard/ppsspp/issues/14256#issuecomment-1136256118).

## Building

The repo has been updated and tested to build with Minimalist PSPSDK 0.10.0 on Windows.
1. Download and install [MINPSPW 0.10.0](https://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.10.0/)
2. Clone the repo and go to `jge/Projects/cspsp`.
3. `make clean` and `make 3xx`, 
4. Copy the created `EBOOT.pbp` to the `bin/` folder.
5. Copy the entire `bin/` folder to `ms0:/PSP/GAME/` on your PSP (and optionally rename the directory to `CSPSP/`).

## Zombie mod

- Infected mode: if you die as CT you will re-spawn as a Zombie
- Extermination: no respawn if you die as a CT (Team Deathmatch)
- Zombies have 150 HP
- Zombies deal 75hp damage with the knife (unline 40hp for CT)
- In the settings menu you can set the re-spawn mode
- You can either respawn in-place (where you have been slain) or you can re-spawn in the T base-
- You can also set a custom delay for re-spawn (for game balancing reasons or maps  that are too tight)
