# Step 9: Postprocessors id=step09

!---

Aggregate values based on simulation data are useful for understanding the simulation as well
as defining coupling values across coupled equations.

There are two main systems for aggregating data: Postprocessors and VectorPostprocessors.

!!end-intro

!---

## Step 9: Input File

!listing step09_postprocessing/step9.i

!---

## Step 9: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step09_postprocessing
moose-opt -i step9.i
```

Then plot from the `CSV` file in python, excel, paraview etc

!---

To have non-zero values at `t=0`, the postprocessors must be executed on `INITIAL`.

```text
Postprocessor Values:
+----------------+--------------------------+----------------+----------------+
| time           | max_temperature_concrete | num_elements   | water_heat_flux|
+----------------+--------------------------+----------------+----------------+
|   0.000000e+00 |             3.000000e+02 |   1.792000e+04 |   0.000000e+00 |
|   4.320000e+04 |             3.226249e+02 |   1.792000e+04 |   5.445889e+03 |
|   8.640000e+04 |             3.348682e+02 |   2.436000e+04 |   7.445028e+03 |
|   1.296000e+05 |             3.416370e+02 |   3.379600e+04 |   8.692277e+03 |
|   1.728000e+05 |             3.464075e+02 |   3.491600e+04 |   9.406888e+03 |
|   2.160000e+05 |             3.500610e+02 |   3.519600e+04 |   9.912671e+03 |
+----------------+--------------------------+----------------+----------------+
```

!---

## Step 9: Visualization using Plotly

!listing step09_postprocessing/step9.py

!---

Ensure the MOOSE python utilities are in the `PYTHONPATH`:

```bash
export PYTHONPATH=$PYTHONPATH:$HOME/projects/moose/python
python step9.py
```

!---

!row!

!col! width=50%

!media step9_max_temp.png

!media step9_Tx.png

!col-end!

!col! width=50%

!media step9_heat_flux.png

!media step9_Ty.png

!col-end!

!row-end!
