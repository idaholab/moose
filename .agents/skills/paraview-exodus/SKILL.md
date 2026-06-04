---
name: paraview-exodus
description: Visualize an ExodusII file with ParaView — render a field with a chosen colormap to a PNG, or, for multi-timestep files, render a frame series and stitch it into an MP4 animation with ffmpeg. Use when asked to visualize, plot, render, or animate an exodus (.e/.exo) file.
---

# Visualize ExodusII files with ParaView

Drives ParaView's Python interface (`paraview.simple`, run via `pvpython`) to
render an ExodusII mesh colored by a field, and `ffmpeg` to turn a multi-timestep
series into a video. Assumes `pvpython` and `ffmpeg` are on the PATH.

## How to use it

The entry point is `visualize_exodus.sh` in this skill's directory. It runs the
renderer and, when the file has multiple timesteps, encodes the frames into MP4.

```bash
# Most basic — color by the first field, default viridis colormap, white background
./visualize_exodus.sh path/to/result.e

# Specify field and colormap
./visualize_exodus.sh result.e --field temperature --colormap coolwarm

# Custom background and resolution
./visualize_exodus.sh result.e --field disp_x --background "#202020" --resolution 1280x720

# Fit the frame to the mesh shape (good for long/thin or wide domains)
./visualize_exodus.sh result.e --field disp --auto-aspect
```

Always run it from this skill's directory (or with an absolute path to the
script) so it can find `render_exodus.py` next to it. Outputs are written to the
current working directory using the input's basename unless `--output` is given.

**Before rendering an unfamiliar file, list its fields** so you pick a real one:

```bash
./visualize_exodus.sh result.e --list-fields
```

## Behavior

- **Single timestep** → one image, `PREFIX.png`.
- **Multiple timesteps** → ParaView writes `PREFIX.0000.png …`, then ffmpeg
  encodes `PREFIX.mp4`. Intermediate frames are deleted unless `--keep-frames`.
- Frames are rendered directly at the requested resolution (default 800×600).
  ffmpeg only rounds dimensions to even numbers (`scale=trunc(iw/2)*2:...`), a
  yuv420p requirement; it does not rescale.
- Color range defaults to **fixed over all timesteps** so colors are comparable
  frame to frame (`--rescale per-frame` to recompute each frame).
- 2D meshes (flat in z) are auto-detected and rendered with parallel projection.
- `--auto-aspect` reshapes the frame so the mesh fills it instead of floating in
  empty background; `--resolution` then acts as the bounding size. It works by a
  **probe pass**: every frame is rendered (off-screen, no legend) and the actual
  mesh silhouette is measured, then unioned across all frames. Because it
  measures rendered pixels rather than reported bounds, it correctly contains
  meshes that **deform or move** over time (e.g. a bending beam) — the frame fits
  the mesh at every timestep, not just the first. Cost: it renders the series
  twice (probe + final). Requires Python PIL/Pillow in `pvpython`; if missing, it
  falls back to the fixed `--resolution`.

## Options

Renderer (`render_exodus.py`), all forwarded by the wrapper:

| Option | Default | Meaning |
| - | - | - |
| `--field NAME` | first available | Field/variable to color by |
| `--component C` | `Magnitude` | Vector component: `Magnitude`, `X`/`Y`/`Z`, or index |
| `--colormap NAME` | `viridis` | Short name or any ParaView preset (see below) |
| `--invert-colormap` | off | Reverse the colormap |
| `--background COLOR` | `white` | `name` \| `#rrggbb` \| `r,g,b` (also `transparent`) |
| `--text-color COLOR` | `auto` | Scalar bar / axis text; `auto` contrasts with the background |
| `--resolution WxH` | `800x600` | Output resolution (bounding size if `--auto-aspect`) |
| `--auto-aspect` | off | Probe all frames; reshape so the mesh fills the frame |
| `--output PREFIX` | input basename | Output name (no extension) |
| `--rescale fixed\|per-frame` | `fixed` | Color range strategy |
| `--no-scalar-bar` | shown | Hide the color legend |
| `--no-edges` | shown | Hide element/mesh edges |
| `--edge-color COLOR` | black | Edge color when edges are shown |
| `--orientation-axes` | off | Show the orientation triad |
| `--list-fields` | — | Print available point/cell fields and exit |

Wrapper-only:

| Option | Default | Meaning |
| - | - | - |
| `--fps N` | `15` | Animation frame rate |
| `--keep-frames` | off | Keep intermediate PNGs |
| `-- <ffmpeg args>` | — | Everything after `--` is passed verbatim to ffmpeg |

## Colormap names

Short aliases: `viridis`, `plasma`, `inferno`, `magma`, `cividis`, `coolwarm`,
`coolwarm-extended`, `jet`, `rainbow`, `turbo`, `blackbody`, `grayscale`, `xray`.
Any other value is passed straight to ParaView's `ApplyPreset`, so full preset
names like `"Cool to Warm (Extended)"` or `"Black-Body Radiation"` also work.

## Notes for the assistant

- If the user names a field, pass it through; if unsure it exists, run
  `--list-fields` first and report the options.
- Vector fields (e.g. displacement) default to magnitude; use `--component` for
  a single component.
- To pass extra ffmpeg flags (codec, quality), append them after `--`, e.g.
  `-- -crf 18 -c:v libx264`.
- Report the final output path(s) to the user when done.
