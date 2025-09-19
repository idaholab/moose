# Step 3: Postprocessing id=ictp_step3

!---

## `Postprocessor` System

The [`PostProcessor`](Postprocessors/index.md) system is used to aggregate data from a simulation.

Common use cases are:

- Volumetric integration of a field

  - Variable averages

- Surface integration of a field on a boundary

  - Flux on a boundary

- Maximum and minimum values of a field
- Other simulation information

  - Eigenvalues
  - Timing information
  - Solver properties

`PostProcessor` values are output in a table to screen and can also be output per timestep and optionally iteration to a CSV file.

!---

## Diffusion Problem Postprocessing

For the simple diffusion problem in the previous step, the [`PostProcessor`](Postprocessors/index.md) system will be used to output the average value of the solution variable $u$ on the outer boundary and within the fuel.

!listing ictp/inputs/step3_postprocessing.i diff=ictp/inputs/step2-1_diffusion.i

!---

## Run: Diffusion Problem Postprocessing

```bash
$ cardinal-opt -i step3_postprocessing.i
```

At the end of the simulation, the postprocessor values appear on screen:

```
Postprocessor Values:
+----------------+----------------+----------------+
| time           | fuel_average   | outer_average  |
+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e+00 |   5.951556e-01 |   1.000000e+00 |
+----------------+----------------+----------------+
```

You may also open the `step3_postprocessing_out.csv` file to view the csv contents.
