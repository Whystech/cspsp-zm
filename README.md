# CSPSP:ZM

![screenshot1](screenshots/ss1.png)
![screenshot2]

CSPSP:ZM is a homebrew game mod for the Sony PSP's CSPSP, created in 2026.

It is a 2d top-down shooter mod based on Kevinbchen's CSPSP. The mod only works offline. 
It has 2 game modes: Infection and Extermination.

This mod is based on Kevinbchen's CSPSP which can be reviewed below.
The game is still available to download - see the [Releases](https://github.com/kevinbchen/cspsp/releases) page.

> **Warning**: A lot of refactoring has been carried throughout, I have tried to comment some of the original code so it is easier to find and edit.

## Playing

Simply copy the game into ms0:/PSP/GAME/ like any other homebrew. 
CSPSP ZM is based on the CSPSP v1.92b release.

## Building

The repo has been updated and tested to build with Minimalist PSPSDK 0.10.0 on Windows.
1. Download and install [MINPSPW 0.10.0](https://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.10.0/)
2. Clone the repo and go to `jge/Projects/cspsp`.
3. `make clean` and `make 3xx`, 
4. Copy the created `EBOOT.pbp` to the `bin/` folder.
5. Copy the entire `bin/` folder to `ms0:/PSP/GAME/` on your PSP (and optionally rename the directory to `CSPSP/`).

## Customization

Client resources and gameplay records are configured under
`jge/Projects/cspsp/bin/data`. See `CONFIGURATION.txt` in that directory for
the complete field and resource reference. Restart the game after changing a
configuration file or replacing an asset.

### Gun Editor

Run `Gun Editor.jar` from `jge/Projects/cspsp` to edit and preview weapon
records. The editor supports weapon sprites, player models, animation profiles,
muzzle positions and flashes, projectile origins, spread, screen shake, bullet
impacts, explosions, guide lines, and hitmarkers. It writes the same text files
used by the PSP game.

`guns.txt` supports up to 128 sequential weapon IDs. Each ID selects a 32x32
cell from the matching hand and ground weapon atlases. The two configured atlas
pages contain 64 weapons each. Weapon fire and reload WAV files are loaded into
RAM, so short mono 22050 Hz samples are recommended when expanding the catalog.

### Player Models

`players.txt` defines each selectable model with this format:

```text
player <network ID> <team ID> <atlas cell> <menu name>
```

Team `0` is Zombies and team `1` is CT. The team ID controls ownership
independently of atlas position. Network IDs must be unique and stable for
multiplayer; atlas cells select the graphics and may be listed in any order.
Live and dead graphics for a model must occupy the same cell in their respective
atlases. Up to 32 models across four eight-model atlas pages are supported.

### Configurable Effects And Audio

- `resources.txt` selects client textures and optional atlas pages.
- `audio.txt` selects global, team, and gameplay sounds.
- `muzzlepositions.txt` and `projectileorigins.txt` control launch positions.
- `screenshakes.txt` configures firing and explosion camera shake.
- `aimmarkers.txt` configures guide lines and hitmarkers.

CT death variants use `sfx/die1.wav` through `sfx/die3.wav`. Zombie death
variants use `sfx/zdie1.wav` through `sfx/zdie3.wav`. Missing Zombie variants
safely fall back to the corresponding CT death sound.

## Zombie mod

- Infected mode: if you die as CT you will re-spawn as a Zombie
- Extermination: no respawn if you die as a CT (Team Deathmatch)
- Zombies have 150 HP
- Zombies deal 75hp damage with the knife (unline 40hp for CT)
- In the settings menu you can set the re-spawn mode
- You can either respawn in-place (where you have been slain) or you can re-spawn in the T base-
- You can also set a custom delay for re-spawn (for game balancing reasons or maps  that are too tight)
