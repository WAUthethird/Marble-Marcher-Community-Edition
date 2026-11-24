# Changelog – MMCE Unofficial Rebuild

Unofficial fork of Marble Marcher: Community Edition 1.4.6. Not affiliated with the MMCE team or CodeParade.

## Unreleased

### Level format
- New extension block after the original level data (old levels load unchanged with defaults)

### Fractal
- Fractal iterations without upper limit; early exit in shader and physics (escape radius / scale limit)
- Configurable rotation axes for Angle1 / Angle2
- Symmetry options: type, mirror axes, global and internal radial symmetry
- *Reacts to music*: the fractal scale pulses to the beat
- Per level fractal glow: color independent of the surface, brightness, pattern, beat sync
- Beat analysis (background thread) with a 5 band sensitivity equalizer and live display

### Sky
- Custom sky color, darkness when the sun is below the horizon, black sky for a black sun

### Gameplay
- Jump with configurable height (Space / gamepad button 0)
- Rubber ball (matte black, restitution, squash & stretch deformation, synthesized bounce sound)
- Ball weight, steering acceleration, max speed, max fall speed, friction from the level, kill Y toggle
- Negative gravity: view rolls upside down, flags are flipped, kill barrier mirrored
- Up to 16 extra flags (ordered multi flag mode: main flag = flag 1) and 16 checkpoints
- Ordered checkpoints mode (arrow shows C1, C2 …, flags locked until all checkpoints are hit)
- Respawn at the last checkpoint or reached flag; R / right click respawn, Shift+R or R twice = restart
- Saved checkpoint progress with a "Start from new / Start from checkpoint" dialog
- Split times; level time limit that destroys the ball
- Crush effects (setting): glass/ceramic shards with physics, metal "dumpling", respawn after 3 s

### Editor
- 7 decimal places, original crank speeds, logarithmic marble radius
- Markers for start / flags / checkpoints, placing exactly under the cursor
- Duplicate level (level menu and editor), reset physics button

### Rendering / performance
- Fixed 60 Hz simulation with interpolated rendering, no hard 60 fps cap, V-Sync applied immediately
- Bounding spheres for marble and flag in the distance estimator, fast path for the default symmetry
- Cached uniform locations, glProgramUniform
- Depth of field scales with the marble size

### Fixes
- Menu / editor fractal flicker
- Countdown sounds playing several times at high frame rates
- Crash with missing level music and with an empty music list in the editor
- Broken `.lvl` files are skipped instead of being loaded as garbage
- Level list click fired every frame while the mouse button was held
- noise test shader did not compile; stale shader error dump removed

### Build
- MSYS2 / clang64 build path in CMake, `tools/setup_deps_msys2.sh`, `tools/package_windows.sh`
