# Troubleshooting

Most mistakes in an input file will cause wrong results,
usually affecting convergence of the solve as well. We cover here two common problems:

- Input file mistakes and how to find them

- Non-convergence of the solver

!---

## Input file mistakes

If a careful review of the input does not find the error,
the next thing to pay attention to is the simulation log.

- Are there any warnings? By default MOOSE will not error on warnings
- Are there any unused parameters? They could be misspelled!

If that does work, it is time to examine how the simulation evolves in MOOSE

!---

## Additional outputs

By default, MOOSE outputs on the end of timesteps

```
[Outputs]
  execute_on = TIMESTEP_END
```

We can change this parameter to output as often as linear iterations!
We make sure to output material properties as well, in case the problem lies there:

```bash
[Outputs]
  [exo]  # filename suffix
    type = Exodus
    execute_on = 'LINEAR TIMESTEP_END'
    output_material_properties = true
  []
[]
```

Add any output you need to understand the root cause!

!---

## Using the [Debug system](syntax/Debug/index.md)

To look for an issue during setup, we can list the objects created by MOOSE for numerous systems. For example, for material properties,

```bash
[Debug]
  show_material_props = true
[]
```

For a general log on the entire setup:

```bash
[Debug]
  show_actions = true
[]
```

!---

To look for an issue during the execution,

```bash
[Debug]
  show_execution_order = ALWAYS
[]
```

This will output to the console, the execution of all MOOSE's objects, in their respective nodal/elemental/side loops on the mesh.

!---

## Troubleshooting failed solves

A comprehensive list of techniques is available in the [documentation](application_usage/failed_solves.md)

First, you should diagnose the non-convergence by printing the residuals for all variables:

```bash
[Debug]
  show_var_residual_norms = true
[]
```

You can then identify which variable is not converging.
Equation scaling issues have been covered earlier. Let's explore two other common causes:

- initialization

- bad mesh

!---

Make sure to initialize every nonlinear variable using the `[ICs]` block.
To check initialization, use the [Exodus](Exodus.md) output:

```bash
[Outputs]
  exodus = true
  execute_on = INITIAL
[]
```

!---

Meshing is hard. We have some tools to help in the [MeshGenerator system](syntax/Mesh/index.md) but generally you should:

- visually inspect your mesh. Look for unsupported features: non-conformality (except from libmesh refinement), overlapping cells, mixed element types within a subdomain (within a mesh is OK)
- use `show_info = true` in the [FileMeshGenerator](FileMeshGenerator.md) and verify that the output is as expected
- replace your mesh with a simple MOOSE-generated rectangular mesh to check if the mesh is at fault

!---

## Summary of helpful resources

[Documentation for every object](syntax/index.md)

[Troubleshooting failed solves](application_usage/failed_solves.md)

[Debug system](syntax/Debug/index.md)

[FAQ](https://mooseframework.inl.gov/help/faq/index.html)

[GitHub discussions forum](https://github.com/idaholab/moose/discussions) : please follow the [guidelines](https://github.com/idaholab/moose/discussions/18270) before posting
