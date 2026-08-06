#!/usr/bin/env pvpython
"""
Render an ExodusII file with ParaView's Python interface (paraview.simple).

- Single timestep  -> one PNG  ($PREFIX.png)
- Multiple timesteps -> a numbered PNG series ($PREFIX.0000.png, ...) suitable
  for stitching into a video with ffmpeg.

Frames are rendered directly at the requested resolution (default 800x600).

Run with pvpython:
    pvpython render_exodus.py mesh.e --field temperature --colormap viridis
"""

import argparse
import os
import sys

from paraview.simple import (
    OpenDataFile,
    Show,
    ColorBy,
    GetActiveViewOrCreate,
    GetColorTransferFunction,
    GetOpacityTransferFunction,
    GetAnimationScene,
    SaveScreenshot,
    SaveAnimation,
    ResetCamera,
    Render,
    GetActiveCamera,
    GetScalarBar,
)

# Camera orientations as (azimuth, elevation) applied to the default front view
# (looking along -z, up +y). 'auto' resolves to 'iso' for 3D, 'front' for 2D.
VIEW_ANGLES = {
    "front": (0, 0),
    "back": (180, 0),
    "right": (90, 0),
    "left": (-90, 0),
    "top": (0, 90),
    "bottom": (0, -90),
    "iso": (-45, 25),
}


# Short, friendly colormap names mapped to ParaView preset names. Anything not
# listed here is passed straight through to ApplyPreset() so the full ParaView
# preset catalog remains available.
COLORMAP_ALIASES = {
    "viridis": "Viridis (matplotlib)",
    "plasma": "Plasma (matplotlib)",
    "inferno": "Inferno (matplotlib)",
    "magma": "Magma (matplotlib)",
    "cividis": "Cividis (matplotlib)",
    "coolwarm": "Cool to Warm",
    "cool-to-warm": "Cool to Warm",
    "coolwarm-extended": "Cool to Warm (Extended)",
    "jet": "Jet",
    "rainbow": "Rainbow Uniform",
    "blackbody": "Black-Body Radiation",
    "grayscale": "Grayscale",
    "xray": "X Ray",
    "turbo": "Turbo",
}

# A handful of named colors for the --background option.
NAMED_COLORS = {
    "white": (1.0, 1.0, 1.0),
    "black": (0.0, 0.0, 0.0),
    "gray": (0.5, 0.5, 0.5),
    "grey": (0.5, 0.5, 0.5),
    "lightgray": (0.83, 0.83, 0.83),
    "lightgrey": (0.83, 0.83, 0.83),
    "transparent": None,
    "paraview": (0.32, 0.34, 0.43),  # the classic ParaView blue-gray
}


def parse_color(text):
    """Parse a color given as a name, '#rrggbb' hex, or 'r,g,b' (0-1 or 0-255)."""
    text = text.strip().lower()
    if text in NAMED_COLORS:
        return NAMED_COLORS[text]
    if text.startswith("#"):
        h = text.lstrip("#")
        if len(h) == 3:
            h = "".join(c * 2 for c in h)
        return tuple(int(h[i : i + 2], 16) / 255.0 for i in (0, 2, 4))
    if "," in text:
        parts = [float(p) for p in text.split(",")]
        if len(parts) != 3:
            raise ValueError("RGB color must have three components: '%s'" % text)
        if any(p > 1.0 for p in parts):  # assume 0-255
            parts = [p / 255.0 for p in parts]
        return tuple(parts)
    raise ValueError("Unrecognized color: '%s'" % text)


def contrasting_color(rgb):
    """Return black or white (whichever contrasts more) for the given 0-1 RGB."""
    r, g, b = rgb
    luminance = 0.299 * r + 0.587 * g + 0.114 * b  # perceived brightness
    return (0.0, 0.0, 0.0) if luminance > 0.5 else (1.0, 1.0, 1.0)


def camera_axes(cam):
    """Return (right, up, view_direction) unit vectors of the camera."""
    import math

    def sub(a, b):
        return (a[0] - b[0], a[1] - b[1], a[2] - b[2])

    def cross(a, b):
        return (
            a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0],
        )

    def dot(a, b):
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]

    def norm(a):
        m = math.sqrt(dot(a, a)) or 1.0
        return (a[0] / m, a[1] / m, a[2] / m)

    pos, fp, up = cam.GetPosition(), cam.GetFocalPoint(), cam.GetViewUp()
    vdir = norm(sub(fp, pos))
    right = norm(cross(vdir, up))
    tup = norm(cross(right, vdir))
    return right, tup, vdir


def auto_frame(view, display, reader, times, w, h, reserve_for_bar):
    """Reshape the frame to fit the mesh across ALL timesteps.

    Renders every frame as a solid silhouette on a saturated background and
    unions the projected bounding boxes (the actual rendered geometry, so mesh
    deformation that the reader's bounds don't report is captured). Then sets
    the viewport and a parallel camera so the union fills the frame, with the
    legend strip kept clear on the right. Returns (w, h, reserve_px), or None to
    fall back to the fixed frame.
    """
    try:
        from PIL import Image, ImageChops
    except Exception:
        print("WARNING: PIL unavailable; --auto-aspect needs it. Using %dx%d." % (w, h))
        return None

    import os
    import tempfile

    cam = GetActiveCamera()
    saved_bg = list(view.Background)
    box_w, box_h = w, h
    PROBE = 520
    probe_bg = (0.0, 1.0, 0.0)  # saturated colour no colormap will produce
    scene = GetAnimationScene()
    frames = times if times else [None]

    def silhouette_bbox(path):
        # Background is sampled from a corner pixel, so the silhouette is found
        # regardless of how the mesh happens to be coloured.
        im = Image.open(path).convert("RGB")
        bg = im.getpixel((0, 0))
        return ImageChops.difference(im, Image.new("RGB", im.size, bg)).getbbox()

    try:
        # Probe on a saturated green background (no colormap produces it), so the
        # mesh silhouette stands out cleanly. The representation is left as-is;
        # edges, if any, fall inside the silhouette and don't change its bbox.
        view.CameraParallelProjection = 1
        view.ViewSize = [PROBE, PROBE]
        view.Background = list(probe_bg)

        right, up, _ = camera_axes(cam)
        ResetCamera(view)
        focal = cam.GetFocalPoint()
        base_scale = cam.GetParallelScale()

        # Zoom out enough that deformed frames stay inside the probe; if any
        # frame still touches an edge it was clipped, so zoom out and re-measure.
        tmp = tempfile.mkstemp(suffix=".png")[1]
        zoom = 1.6
        union = None
        for _attempt in range(5):
            cam.SetParallelScale(base_scale * zoom)
            left = top = 10**9
            rightmost = bottom = -1
            clipped = False
            for t in frames:
                if t is not None:
                    scene.AnimationTime = t
                Render(view)
                SaveScreenshot(tmp, view, ImageResolution=[PROBE, PROBE])
                bb = silhouette_bbox(tmp)
                if bb:
                    left, top = min(left, bb[0]), min(top, bb[1])
                    rightmost, bottom = max(rightmost, bb[2]), max(bottom, bb[3])
                    if (
                        bb[0] <= 1
                        or bb[1] <= 1
                        or bb[2] >= PROBE - 1
                        or bb[3] >= PROBE - 1
                    ):
                        clipped = True
            if rightmost <= left or bottom <= top:
                return None
            union = (left, top, rightmost, bottom, base_scale * zoom)
            if not clipped:
                break
            zoom *= 1.8
        try:
            os.remove(tmp)
        except Exception:
            pass

        left, top, rightmost, bottom, scale_used = union
        wpp = 2.0 * scale_used / PROBE  # world units per probe pixel (square)
        world_w = (rightmost - left) * wpp
        world_h = (bottom - top) * wpp
        cx, cy = (left + rightmost) / 2.0, (top + bottom) / 2.0
        center = [
            focal[i]
            + (cx - PROBE / 2.0) * wpp * right[i]
            + (PROBE / 2.0 - cy) * wpp * up[i]
            for i in range(3)
        ]

        aspect = world_w / world_h
        w, h, reserve_px = fit_resolution(box_w, box_h, aspect, reserve_for_bar)
        geo_w = max(2, w - reserve_px)
        margin = 0.95
        wpp_f = max(world_h / (margin * h), world_w / (margin * geo_w))
        scale_f = wpp_f * h / 2.0

        # Centre the camera on the union, keep orientation/distance, set scale.
        view.ViewSize = [w, h]
        pos = cam.GetPosition()
        delta = [center[i] - cam.GetFocalPoint()[i] for i in range(3)]
        cam.SetFocalPoint(*center)
        cam.SetPosition(*(pos[i] + delta[i] for i in range(3)))
        cam.SetParallelScale(scale_f)
        if reserve_px:  # shift mesh left so the right strip is free for the legend
            shift = 0.5 * reserve_px * wpp_f
            p, f = cam.GetPosition(), cam.GetFocalPoint()
            cam.SetPosition(*(p[i] + shift * right[i] for i in range(3)))
            cam.SetFocalPoint(*(f[i] + shift * right[i] for i in range(3)))

        print(
            "Auto aspect: %dx%d (mesh aspect %.2f, %d frames probed)"
            % (w, h, aspect, len(frames))
        )
        return w, h, reserve_px
    except Exception as exc:
        print("WARNING: auto-aspect probe failed (%s); using fixed frame." % exc)
        return None
    finally:
        view.Background = saved_bg


# Pixels reserved on the right for a vertical color legend + its tick labels.
SCALAR_BAR_RESERVE_PX = 130


def fit_resolution(box_w, box_h, aspect, reserve_for_bar):
    """Fit `aspect` (w/h) inside the box_w x box_h bounding size; pad width for a
    vertical scalar bar. Returns even (w, h, reserve_px) for video encoding;
    reserve_px is the width set aside on the right for the color legend."""
    if aspect >= box_w / box_h:
        w, h = box_w, box_w / aspect
    else:
        w, h = box_h * aspect, box_h
    reserve = 0.0
    if reserve_for_bar:
        # Enough room for the bar + labels even when the geometry is narrow.
        reserve = max(0.18 * w, SCALAR_BAR_RESERVE_PX)
        w += reserve
    w, h = int(round(w)), int(round(h))
    w += w % 2  # yuv420p needs even dimensions
    h += h % 2
    return max(2, w), max(2, h), int(round(reserve))


def list_arrays(reader):
    """Return dicts of {name: num_components} for point and cell arrays."""
    info = reader.GetDataInformation()
    point = {}
    cell = {}
    pdi = info.GetPointDataInformation()
    for i in range(pdi.GetNumberOfArrays()):
        a = pdi.GetArrayInformation(i)
        point[a.GetName()] = a.GetNumberOfComponents()
    cdi = info.GetCellDataInformation()
    for i in range(cdi.GetNumberOfArrays()):
        a = cdi.GetArrayInformation(i)
        cell[a.GetName()] = a.GetNumberOfComponents()
    return point, cell


def enable_all_variables(reader):
    """Try to switch on all available exodus variables so fields are loadable."""
    for prop in (
        "PointVariables",
        "ElementVariables",
        "NodeSetArrayStatus",
        "SideSetArrayStatus",
        "GlobalVariables",
    ):
        try:
            sel = getattr(reader, prop)
            avail = list(sel.Available)
            if avail:
                setattr(reader, prop, avail)
        except Exception:
            pass


def main():
    p = argparse.ArgumentParser(description="Render an ExodusII file with ParaView.")
    p.add_argument("input", help="Path to the ExodusII file (.e, .exo, ...).")
    p.add_argument(
        "--field",
        help="Field/variable to color by. "
        "If omitted, the first available field is used.",
    )
    p.add_argument(
        "--component",
        default="Magnitude",
        help="Component for vector/tensor fields: 'Magnitude' (default), "
        "'X', 'Y', 'Z', or an integer index.",
    )
    p.add_argument(
        "--colormap",
        default="viridis",
        help="Colormap: short name (viridis, coolwarm, jet, ...) or any "
        "ParaView preset name. Default: viridis.",
    )
    p.add_argument(
        "--invert-colormap", action="store_true", help="Invert/reverse the color map."
    )
    p.add_argument(
        "--background",
        default="white",
        help="Background color: name, #rrggbb, or 'r,g,b'. Default: white.",
    )
    p.add_argument(
        "--text-color",
        default="auto",
        help="Color for the scalar bar and axis text. Default 'auto' "
        "picks black or white to contrast with the background.",
    )
    p.add_argument(
        "--resolution",
        default="800x600",
        help="Output image/video resolution WxH. Default: 800x600.",
    )
    p.add_argument(
        "--auto-aspect",
        action="store_true",
        help="Adjust the aspect ratio to fit the mesh's bounding box "
        "(reduces wasted background), keeping --resolution as the "
        "bounding size.",
    )
    p.add_argument(
        "--output",
        help="Output prefix (no extension). "
        "Default: input filename without extension.",
    )
    p.add_argument(
        "--rescale",
        choices=["fixed", "per-frame"],
        default="fixed",
        help="Color range: 'fixed' over all timesteps (default) or " "'per-frame'.",
    )
    p.add_argument(
        "--scalar-bar",
        dest="scalar_bar",
        action="store_true",
        default=True,
        help="Show the color legend (default).",
    )
    p.add_argument(
        "--no-scalar-bar",
        dest="scalar_bar",
        action="store_false",
        help="Hide the color legend.",
    )
    p.add_argument(
        "--edges",
        dest="edges",
        action="store_true",
        default=True,
        help="Overlay element edges (default).",
    )
    p.add_argument(
        "--no-edges",
        dest="edges",
        action="store_false",
        help="Do not overlay element edges.",
    )
    p.add_argument(
        "--orientation-axes",
        action="store_true",
        help="Show the orientation axes triad (off by default).",
    )
    p.add_argument(
        "--view",
        default="auto",
        choices=["auto", "front", "back", "left", "right", "top", "bottom", "iso"],
        help="Camera orientation. 'auto' (default) uses an isometric "
        "view for 3D meshes and a front view for 2D meshes.",
    )
    p.add_argument(
        "--edge-color",
        default="0,0,0",
        help="Edge color when --edges is on. Default: black.",
    )
    p.add_argument(
        "--list-fields", action="store_true", help="List available fields and exit."
    )
    args = p.parse_args()

    if not os.path.isfile(args.input):
        sys.exit("ERROR: input file not found: %s" % args.input)

    try:
        w, h = (int(x) for x in args.resolution.lower().split("x"))
    except Exception:
        sys.exit("ERROR: --resolution must look like 800x600")

    prefix = args.output or os.path.splitext(os.path.basename(args.input))[0]

    reader = OpenDataFile(args.input)
    if reader is None:
        sys.exit("ERROR: ParaView could not open %s" % args.input)
    enable_all_variables(reader)
    reader.UpdatePipeline()

    point_arrays, cell_arrays = list_arrays(reader)

    if args.list_fields:
        print("Point (nodal) fields:")
        for n, c in sorted(point_arrays.items()):
            print("  %s (%d comp)" % (n, c))
        print("Cell (elemental) fields:")
        for n, c in sorted(cell_arrays.items()):
            print("  %s (%d comp)" % (n, c))
        return

    # Resolve the field and its association.
    field = args.field
    if field is None:
        if point_arrays:
            field, assoc = sorted(point_arrays)[0], "POINTS"
        elif cell_arrays:
            field, assoc = sorted(cell_arrays)[0], "CELLS"
        else:
            sys.exit("ERROR: no point or cell fields found in %s" % args.input)
        print("No --field given; using '%s' (%s)." % (field, assoc))
    elif field in point_arrays:
        assoc = "POINTS"
    elif field in cell_arrays:
        assoc = "CELLS"
    else:
        avail = sorted(point_arrays) + sorted(cell_arrays)
        sys.exit(
            "ERROR: field '%s' not found. Available: %s"
            % (field, ", ".join(avail) or "(none)")
        )

    view = GetActiveViewOrCreate("RenderView")
    view.ViewSize = [w, h]
    view.OrientationAxesVisibility = 1 if args.orientation_axes else 0

    # Background color (override the color palette so it actually takes effect).
    bg = parse_color(args.background)
    for attr, val in (
        ("UseColorPaletteForBackground", 0),
        ("BackgroundColorMode", "Single Color"),
    ):
        try:
            setattr(view, attr, val)
        except Exception:
            pass
    # bg is None for a transparent export; the rendered fill is then white, so
    # contrast text against white in that case.
    bg_fill = (1.0, 1.0, 1.0) if bg is None else bg
    if bg is None:
        try:
            view.UseColorPaletteForBackground = 0
        except Exception:
            pass
        view.Background = list(bg_fill)
    else:
        view.Background = list(bg)

    # Text color: contrast with the background unless explicitly set.
    if args.text_color.lower() == "auto":
        text_color = contrasting_color(bg_fill)
    else:
        text_color = parse_color(args.text_color)
    # Axis/triad labels (visible only when --orientation-axes is on).
    try:
        view.OrientationAxesLabelColor = list(text_color)
    except Exception:
        pass

    display = Show(reader, view)
    display.Representation = "Surface With Edges" if args.edges else "Surface"
    if args.edges:
        try:
            display.EdgeColor = list(parse_color(args.edge_color))
        except Exception:
            pass

    # Detect 2D meshes (flat in z) and use parallel projection for a clean view.
    bounds = reader.GetDataInformation().GetBounds()
    dx, dy, dz = (bounds[1] - bounds[0], bounds[3] - bounds[2], bounds[5] - bounds[4])
    extent = max(dx, dy, dz, 1e-30)
    is_2d = dz / extent < 1e-6
    if is_2d:
        try:
            view.InteractionMode = "2D"
            view.CameraParallelProjection = 1
        except Exception:
            pass

    times = list(reader.TimestepValues) if reader.TimestepValues else []

    # Orient the camera. 'auto' -> iso for 3D, front for 2D.
    view_name = args.view
    if view_name == "auto":
        view_name = "front" if is_2d else "iso"
    azimuth, elevation = VIEW_ANGLES.get(view_name, (0, 0))

    def orient_camera():
        # ResetCamera only repositions the camera along its current direction
        # (it does NOT reset rotation), so apply azimuth/elevation only once.
        ResetCamera(view)
        if azimuth or elevation:
            try:
                cam = GetActiveCamera()
                if azimuth:
                    cam.Azimuth(azimuth)
                if elevation:
                    cam.Elevation(elevation)
                cam.OrthogonalizeViewUp()
                ResetCamera(view)
            except Exception:
                pass

    orient_camera()

    # Optionally reshape the frame to fit the mesh across all frames. The probe
    # pass measures the actual rendered silhouette (so mesh deformation is
    # captured), which the reader's reported bounds do not include.
    if args.auto_aspect:
        framed = auto_frame(view, display, reader, times, w, h, bool(args.scalar_bar))
        if framed:
            w, h, _reserve = framed
        else:  # probe bailed out (e.g. no PIL); keep a sane fixed frame
            view.ViewSize = [w, h]
            orient_camera()
        if times:  # the probe advances the reader's time; restore it
            reader.UpdatePipeline(times[0])

    # Color by the chosen field/component.
    ColorBy(display, (assoc, field))
    ctf = GetColorTransferFunction(field)
    otf = GetOpacityTransferFunction(field)

    # Vector component selection.
    comp = args.component
    try:
        if comp.lower() == "magnitude":
            ctf.VectorMode = "Magnitude"
        else:
            ctf.VectorMode = "Component"
            idx = {"x": 0, "y": 1, "z": 2}.get(comp.lower())
            ctf.VectorComponent = idx if idx is not None else int(comp)
    except Exception:
        pass

    # Apply the colormap preset.
    preset = COLORMAP_ALIASES.get(args.colormap.lower(), args.colormap)
    try:
        ctf.ApplyPreset(preset, True)
    except Exception:
        print("WARNING: could not apply colormap preset '%s'; using default." % preset)
    if args.invert_colormap:
        try:
            ctf.InvertTransferFunction()
        except Exception:
            pass

    display.SetScalarBarVisibility(view, bool(args.scalar_bar))
    if args.scalar_bar:
        try:
            sb = GetScalarBar(ctf, view)
            sb.TitleColor = list(text_color)
            sb.LabelColor = list(text_color)
            # Keep the legend a vertical bar on the right; AutoOrient otherwise
            # flips it horizontal (and dumps it at the bottom) in narrow frames.
            sb.AutoOrient = 0
            sb.Orientation = "Vertical"
            sb.WindowLocation = "Lower Right Corner"
            sb.ScalarBarLength = 0.6
            sb.ScalarBarThickness = 14
        except Exception:
            pass

    Render(view)

    if len(times) <= 1:
        # Single timestep -> one still image.
        if args.rescale == "fixed" or True:
            display.RescaleTransferFunctionToDataRange(False, True)
        out = prefix + ".png"
        SaveScreenshot(
            out,
            view,
            ImageResolution=[w, h],
            TransparentBackground=1 if bg is None else 0,
        )
        print("STILL %s" % out)
        return

    # Multiple timesteps -> animation frame series.
    scene = GetAnimationScene()
    scene.UpdateAnimationUsingDataTimeSteps()

    if args.rescale == "fixed":
        # Range over the entire time series, so colors are comparable per frame.
        display.RescaleTransferFunctionToDataRangeOverTime()
    else:
        scene.AnimationTime = times[0]
        display.RescaleTransferFunctionToDataRange(False, True)

    out = prefix + ".png"  # ParaView writes prefix.0000.png, prefix.0001.png, ...
    SaveAnimation(out, view, ImageResolution=[w, h], FrameWindow=[0, len(times) - 1])
    print("ANIMATION %s frames=%d resolution=%dx%d" % (prefix, len(times), w, h))


if __name__ == "__main__":
    main()
