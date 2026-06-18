# Dam-Break Comparison Scaling

This note records the nondimensional scaling used by the Martin-Moyce dam-break
comparison inputs. Use these definitions when reconstructing comparison data
from Exodus output or external CSV files.

## Geometry

For the `n^2 = 1`, `a = 2.25 in` Martin-Moyce case:

- `a_length = 0.05715`
- `dam_x = a_length` for the OpenFOAM reference case copied from Rod
- `dam_y = n_squared_value * a_length`
- `n_squared_value = 1`

The OpenFOAM reference case initializes the liquid column as
`0 <= x < a` and `0 <= y < a`:

```text
box (0 0 -1) (0.05715 0.05715 1)
```

Do not compare that OpenFOAM curve against a MOOSE case initialized with
`dam_x = 2 * a_length`; that is a different dam-break geometry.

## Interface Extraction

Use the `alpha = 0.5` interface location. The historical postprocessors used:

- surge front: maximum `x` crossing in the bottom strip,
  `0 <= y <= 2.5 * cell_dy`
- left-wall/back-column height: maximum `y` crossing in the left strip,
  `0 <= x <= 2.5 * cell_dx`

This matches the old `SubcellInterfacialPosition` setup:

```text
paper_front_position_x_raw:
  direction = x
  extremum_type = max
  threshold = 0.5
  secondary_min = 0
  secondary_max = 2.5 * cell_dy

paper_top_height_y_raw:
  direction = y
  extremum_type = max
  threshold = 0.5
  secondary_min = 0
  secondary_max = 2.5 * cell_dx
```

The Rod OpenFOAM extraction used the same left-wall/back-column height
definition. Reconstructing its raw `alpha.water` field at `t = 0.05` with
per-column `alpha = 0.5` interpolation in the left strip gives
`top_H = 0.9118725616059024`, exactly the value stored in the OpenFOAM
comparison CSV.

## Nondimensional Values

The comparison CSVs should use:

```text
tau = t * sqrt(g / a)
front_Z = x_front / a
top_H = y_top / (a * n_squared_value)
```

For the `n^2 = 1` case, `top_H = y_top / a`.

The OpenFOAM reference front starts at `x_front = a`, so the comparison value
starts at `front_Z = 1.0` by direct division. A MOOSE case using
`dam_x = 2 * a_length` can also be made to start at `1.0` by using
`(x_front - a) / a`, but that scaling hides a geometry mismatch and is not
comparable to the Rod OpenFOAM reference.

There is no analogous offset in the height scaling. A height curve starts at
`1.0` by direct division when its measured location is initially under the flat
column top. Matching the initial value is not enough to prove that two height
curves use the same spatial definition.

## Quick Check

For `a = 0.05715`, if the OpenFOAM-reference reconstructed front is:

```text
x_front = 0.08361975379794164
```

then the comparison value is:

```text
front_Z = 0.08361975379794164 / 0.05715 = 1.463302761092592
```

A `2a`-wide MOOSE case can produce a front value near this range with a
different offset formula, but it is not the same OpenFOAM-reference geometry.

## Cleanup Benchmark Gate

Use `dam_break_openfoam_geometry.i` as the cleanup regression gate for the
sharp-interface path. The input matches the Rod OpenFOAM geometry:

```text
a = 0.05715 m
domain = 10a x 1.25a
mesh = 400 x 50
initial water box = [0, a] x [0, a]
```

Run the full accepted case with:

```bash
CONDA_PREFIX=/Users/chowr/miniforge/envs/moose \
PATH=/Users/chowr/miniforge/envs/moose/bin:$PATH \
LIBMESH_DIR=/Users/chowr/miniforge/envs/moose \
WASP_DIR=/Users/chowr/miniforge/envs/moose \
PYTHON=/Users/chowr/miniforge/envs/moose/bin/python \
./modules/navier_stokes/navier_stokes-opt \
  -i dam_break_openfoam_geometry.i \
  Executioner/end_time=0.4 \
  Outputs/file_base=/private/tmp/dam_break_openfoam_geometry/run_t04 \
  Outputs/console=false
```

Then run the gate:

```bash
CONDA_PREFIX=/Users/chowr/miniforge/envs/moose \
PATH=/Users/chowr/miniforge/envs/moose/bin:$PATH \
/Users/chowr/miniforge/envs/moose/bin/python \
  scripts/compare_dam_break_run_t04.py --check --no-plot
```

The checker writes the comparison CSVs and verifies:

- final MOOSE time reaches `t = 0.4`
- `total_alpha` relative drift is at most `1e-10`
- `alpha` remains bounded within `[-1e-12, 1.000001]`
- accepted MOOSE front/back-height points at `t = 0.1`, `0.2`, and `0.4`
- OpenFOAM reconstruction sanity points at `t = 0.1` and `0.2`
- MOOSE-vs-OpenFOAM and MOOSE-vs-Martin-Moyce MAE limits

The checker uses the local experimental reference file:

```text
/Users/chowr/Downloads/experimental_data/mm_consistency_check_all_data.csv
```

and the local OpenFOAM fields under:

```text
/Users/chowr/Work/projects/freeSurface/rod_output
```

For cleanup work, a change should not be considered accepted until this gate
passes or the accepted benchmark values are deliberately updated with a clear
reason.
