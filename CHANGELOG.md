# Changelog

## 1.0.1 – Unofficial Rebuild
### New
- **Unlimited checkpoints**: `K` sets a checkpoint at the marble position, `L` respawns at the last one, `J` deletes the last one. Checkpoints are stored in a list without a fixed cap. Falling below the kill height or getting crushed respawns at the last checkpoint instead of restarting the level. The number of checkpoints is shown in the HUD.
- **Multi color fractal glow**: new settings *Fractal glow mode* (Original, Multi color structure, Multi color animated, Structure + animated – default) and *Fractal glow intensity*. Structure mode lets three different structural bands of the fractal glow in their own colors; animated mode cycles the glow through the spectrum over time and space.
- **Browser version** (`web/`): standalone WebGL2 port with all 30 levels, desktop physics, touch controls for Android, checkpoints, multi color glow, best times and installable PWA (offline capable).
- `tools/export_levels_web.py` embeds the `.lvl` levels into `web/index.html`, so the page also works as a single file.

### Notes
- The settings file gets two new fields, so existing settings are reset to defaults once on first start.
- Respawning at a checkpoint is not part of the input recording, so such runs are not saved as highscore or replay.
- A native Android app (APK) is not included: the desktop renderer needs OpenGL 4.3 compute shaders, which Android/WebGL do not offer. On Android, use the browser version (it can be installed to the home screen).
