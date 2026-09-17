# ParaView Exodus Rendering

`render_exodus.py` renders an [ExodusII](Exodus.md) file with ParaView's Python
interface, coloring the mesh by a chosen field. A file holding a single timestep
produces one PNG; a file holding several produces a numbered PNG series.
`visualize_exodus.sh` wraps the renderer and encodes that series into an MP4 with
`ffmpeg`. Both live in `python/paraview_exodus`.

## Standard Usage

The renderer runs under `pvpython`, which ParaView installs alongside its
application:

```text
pvpython python/paraview_exodus/render_exodus.py result.e --field temperature
```

To list the fields a file holds before choosing one:

```text
pvpython python/paraview_exodus/render_exodus.py result.e --list-fields
```

The wrapper takes the same options, adds `--fps` and `--keep-frames`, and locates
the renderer next to itself, so it may be invoked from any directory:

```text
python/paraview_exodus/visualize_exodus.sh result.e --field temperature --colormap coolwarm
```

Outputs are written to the current working directory, named after the input file
unless `--output` is given. `-h` lists every option, including the colormap
selection, background and text colors, resolution, and the color range strategy
used across a time series.

## Frame Fitting

`--auto-aspect` reshapes the frame so the mesh fills it, treating `--resolution`
as a bounding size. Every frame is rendered off screen first and the mesh
silhouette is measured from those pixels, so the final frame contains the mesh at
every timestep, including meshes that deform or move over time. This renders the
series twice and requires Pillow in the `pvpython` interpreter; without Pillow the
fixed `--resolution` is used.
