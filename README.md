# Marble Marcher: Community Edition – Unofficial Rebuild

> [!IMPORTANT]
> **This is an unofficial fork.** It is **not** the official *Marble Marcher: Community Edition*
> and is **not affiliated with or endorsed by** the MMCE team, the Marble Marcher community or
> CodeParade. Please do not report problems of this version to the official MMCE project.
>
> **Dies ist eine inoffizielle Version.** Sie ist **nicht** die offizielle *Marble Marcher:
> Community Edition* und steht in **keiner Verbindung** zum MMCE-Team oder zu CodeParade.

Based on [Marble Marcher: Community Edition 1.4.6](https://github.com/WAUthethird/Marble-Marcher-Community-Edition),
which is based on the original [Marble Marcher](https://github.com/HackerPoet/MarbleMarcher) by
[HackerPoet / CodeParade](https://www.youtube.com/channel/UCrv269YwJzuZL3dH5PCgxUw).
All credit for the game itself goes to them. The original MMCE readme (credits, controls, FAQ)
is kept in [docs/README_MMCE_ORIGINAL.md](docs/README_MMCE_ORIGINAL.md).

Licensed under the **GNU GPL v2 or later**, like the original (see [LICENSE](LICENSE)).

## What is new

Everything is configured per level in the level editor (press **F4** to show the editor windows).
Old `.lvl` files keep working; new levels append an extension block.

**Fractal**
- Unlimited fractal iterations: the shader and the physics stop as soon as more iterations
  can not change the result, so even huge values stay fast
- Symmetry: type (cubic/Menger, tetrahedral/Sierpinski, diagonal, mirrors only), mirror axes
  X/Y/Z, global and internal radial symmetry (number of axes)
- Animation: adjustable rotation axes, *Reacts to music* (the fractal pulses to the beat)
- Fractal glow per level: own color (independent of the surface color), brightness, pattern,
  beat sync; **multi color glow** (three colors by structure, animated rainbow or both),
  also forced for every level with the setting *Fractal glow colors*
- Beat detection with a 5 band sensitivity equalizer and a live beat display
- 7 decimal places for all parameters, logarithmic marble scale slider

**Sun and sky**
- Custom sky color; sun below the horizon = completely dark; black sun = black sky

**Game rules**
- Jump (Space / gamepad A) with adjustable height
- Rubber ball: matte black, bouncy, deforms on impact, own "boing" sound, survives being squeezed
- Ball weight, steering acceleration, max speed, max fall speed, friction, kill Y on/off
- Negative gravity turns the view upside down
- Multiple flags in a fixed order (the arrow shows the number of the next flag) and checkpoints
  (**no limit**), optionally in a fixed order as well
- Respawn at the last checkpoint / flag (**R** or right mouse button; **Shift+R** or **R** twice = full restart)
- Checkpoint progress is saved; when the level is started again you can continue from the checkpoint
- Split times and a level time limit (when it runs out the ball is destroyed)
- Optional crush effects: a crushed glass ball breaks into physical shards, a metal ball is
  squashed like a dumpling (setting *Crush effects*)

**Editor**
- Markers for start, flags and checkpoints in the 3D view
- Objects are placed exactly under the cursor (also for tiny marbles)
- Duplicate levels (level menu *Copy*, editor *Duplicate level*)
- *Reset marble physics to defaults*

**Performance and fixes**
- No fixed 60 fps cap anymore: the physics runs at a fixed 60 Hz, the picture is rendered as fast
  as the frame limit / V-Sync allows and interpolated between the physics ticks
- Faster distance estimator (bounding spheres, fast path), cached shader uniforms
- Depth of field scales with the marble size (no full screen blur for tiny marbles)
- Several crash fixes (missing music, broken level files, …), see [CHANGELOG.md](CHANGELOG.md)

**Browser / Android**
- `web/index.html` is a standalone WebGL2 version that runs in any current browser, also on
  Android phones (touch controls, installable as an app). See below.

A German list of all changes: [docs/AENDERUNGEN.md](docs/AENDERUNGEN.md).

## Browser / Android version

The folder [`web/`](web/) contains a standalone WebGL2 version of the game. `web/index.html`
runs on its own (the levels are embedded); no build step is needed.

- Desktop: open `web/index.html`, or serve the folder (`cd web && python3 -m http.server 8000`,
  then `http://localhost:8000`)
- Android: host the folder over https (e.g. GitHub Pages), open it in Chrome and use
  *Add to home screen* – it then starts fullscreen and works offline
- Controls: WASD/arrows + mouse, **K** set checkpoint (unlimited), **L** back to the last one,
  **J** delete the last one, **R** restart; touch: left half joystick, right half camera,
  two fingers zoom, buttons for checkpoints and restart
- The renderer is a simplified single pass ray marcher (no GI, bloom or TXAA) so it runs on
  phones; lower the resolution in the menu if it is slow. The level editor features of the
  rebuild (extra flags, symmetry, …) are not part of the browser version
- After changing levels run `python3 tools/export_levels_web.py` to update the embedded levels

## Building (Windows, MSYS2 / clang)

1. Install [MSYS2](https://www.msys2.org) and open the **MSYS2 CLANG64** shell.
2. Build the dependencies (SFML 2.6.2 and AntTweakBar 1.16 with small compatibility patches,
   everything goes into `./deps`):
   ```bash
   ./tools/setup_deps_msys2.sh
   ```
3. Build the game:
   ```bash
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```
4. Copy the needed DLLs next to the executable and start `build/MarbleMarcher.exe`:
   ```bash
   ./tools/package_windows.sh
   ```

The original Visual Studio instructions are in [docs/build_on_windows_MSVC.md](docs/build_on_windows_MSVC.md).
A graphics card with OpenGL 4.3 (compute shaders) is required.

## Kurzfassung (Deutsch)

Inoffizieller Umbau von MMCE 1.4.6 mit vielen neuen Level-Editor-Optionen (Symmetrie,
Rotationsachsen, Glow mit Beat-Sync und Equalizer, Himmel/Sonne, Sprung, Flummi-Ball,
mehrere Flaggen und beliebig viele Checkpoints, Zeitlimit, Zerquetsch-Effekte, mehrfarbiger Glow)
und ohne 60-fps-Grenze. Dazu eine Browser-Version (`web/`), die auch auf Android läuft.
Bauen: siehe oben; alle Änderungen stehen in [docs/AENDERUNGEN.md](docs/AENDERUNGEN.md).
