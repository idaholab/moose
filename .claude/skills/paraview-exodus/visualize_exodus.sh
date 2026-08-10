#!/usr/bin/env bash
#
# Visualize an ExodusII file with ParaView.
#   - single timestep   -> a PNG
#   - multiple timesteps -> a PNG series rendered by ParaView, stitched into an
#                           MP4 by ffmpeg.
#
# Usage:
#   visualize_exodus.sh INPUT.e [options]
#
# Options (passed through to render_exodus.py unless noted):
#   --field NAME            field/variable to color by (default: first available)
#   --colormap NAME         viridis|coolwarm|jet|... or any ParaView preset
#   --invert-colormap       reverse the colormap
#   --component C           Magnitude|X|Y|Z|<index> for vector fields
#   --background COLOR      name | #rrggbb | r,g,b   (default: white)
#   --resolution WxH        output resolution (default: 800x600)
#   --auto-aspect           reshape aspect ratio to the mesh bounding box
#   --output PREFIX         output basename (default: input name)
#   --rescale fixed|per-frame   color range (default: fixed over all timesteps)
#   --no-scalar-bar         hide the color legend
#   --no-edges              hide element edges
#   --edge-color COLOR      edge color (default: black)
#   --orientation-axes      show the orientation triad
#   --list-fields           list available fields and exit
#   --fps N                 video frame rate           [wrapper, default: 15]
#   --keep-frames           keep the intermediate PNGs  [wrapper]
#   --                      anything after -- is passed verbatim to ffmpeg
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RENDER_PY="${SCRIPT_DIR}/render_exodus.py"

FPS=15
KEEP_FRAMES=0
FFMPEG_EXTRA=()
PASS_ARGS=()

# Split our wrapper-only flags from the args forwarded to render_exodus.py.
# Everything after a literal "--" goes to ffmpeg.
while [[ $# -gt 0 ]]; do
  case "$1" in
    --fps)          FPS="$2"; shift 2 ;;
    --keep-frames)  KEEP_FRAMES=1; shift ;;
    --)             shift; FFMPEG_EXTRA=("$@"); break ;;
    *)              PASS_ARGS+=("$1"); shift ;;
  esac
done

if [[ ${#PASS_ARGS[@]} -eq 0 ]]; then
  echo "Usage: visualize_exodus.sh INPUT.e [options]   (see header for options)" >&2
  exit 2
fi

echo ">> Rendering frames with pvpython..." >&2
RENDER_OUT="$(pvpython "${RENDER_PY}" "${PASS_ARGS[@]}")"
echo "${RENDER_OUT}"

# render_exodus.py prints a status line we can act on.
STATUS_LINE="$(echo "${RENDER_OUT}" | grep -E '^(STILL|ANIMATION)' | tail -1 || true)"

case "${STATUS_LINE}" in
  STILL\ *)
    echo ">> Single image written: ${STATUS_LINE#STILL }" >&2
    ;;
  ANIMATION\ *)
    # ANIMATION <prefix> frames=N resolution=WxH
    read -r _ PREFIX FRAMES RES <<<"${STATUS_LINE}"
    RES="${RES#resolution=}"
    W="${RES%x*}"; H="${RES#*x}"
    echo ">> Encoding ${FRAMES#frames=} frames -> ${PREFIX}.mp4 at ${W}x${H}, ${FPS} fps" >&2
    # Frames are already at the final size; only round dimensions to even
    # numbers (a yuv420p requirement). yuv420p for broad player compatibility.
    ffmpeg -y -framerate "${FPS}" ${FFMPEG_EXTRA[@]+"${FFMPEG_EXTRA[@]}"} \
      -i "${PREFIX}.%04d.png" \
      -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" \
      -pix_fmt yuv420p "${PREFIX}.mp4"
    echo ">> Wrote ${PREFIX}.mp4" >&2
    if [[ "${KEEP_FRAMES}" -eq 0 ]]; then
      rm -f "${PREFIX}".[0-9][0-9][0-9][0-9].png
      echo ">> Cleaned up intermediate frames (use --keep-frames to keep them)." >&2
    fi
    ;;
  *)
    # e.g. --list-fields, or an error already reported by render_exodus.py
    ;;
esac
