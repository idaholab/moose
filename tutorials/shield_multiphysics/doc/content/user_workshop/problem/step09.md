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
cd ~/projects/moose/tutorials/shield_multiphysics/step09_postprocessing
make -j 12 # use number of processors for your system
cd inputs
../moose-opt -i step9.i
```

Then plot from the `CSV` file in python, excel, paraview etc

!---

To have non-zero values at `t=0`, the postprocessors must be executed on INITIAL.

```text
----------------+---------------------+----------------+----------------+
| time           | average_temperature | num_elements   | water_heat_flux|
+----------------+---------------------+----------------+----------------+
|   0.000000e+00 |        0.000000e+00 |   1.569000e+04 |   0.000000e+00 |
|   1.000000e+00 |        2.494313e+02 |   1.569000e+04 |   8.585923e+03 |
|   2.000000e+00 |        3.011852e+02 |   1.569000e+04 |   4.890058e+04 |
|   3.000000e+00 |        3.159797e+02 |   1.569000e+04 |   5.743239e+04 |
|   4.000000e+00 |        3.207428e+02 |   1.569000e+04 |   5.976842e+04 |
+----------------+---------------------+----------------+----------------+
```
