# Restart and Recovery System

!---

## Definitions

+Restart+\\
Running a simulation that uses data from a previous simulation, using different input files

+Recover+\\
Resuming an existing simulation after a premature termination

+Solution file+\\
A mesh format containing field data in addition to the mesh (i.e. a normal output file)

+Checkpoint+\\
A snapshot of the simulation including all meshes, solutions, and stateful data

+N to N+\\
In a restart context, this means the number of processors for the previous and current simulations match

+N to M+\\
In a restart context, different numbers of processors may be used for the previous and current simulations

!---

## Variable Initialization

This method is best suited for restarting a simulation when the mesh in the previous simulation
exactly matches the mesh in the current simulation and only initial conditions need to be set for one
more variables.

- This method requires only a valid solution file
- MOOSE supports N to M restart when using this method

!---

```text
[Mesh]
  # MOOSE supports reading field data from ExodusII, XDA/XDR, and mesh checkpoint files (.e, .xda, .xdr, .cp)
  file = previous.e
  # This method of restart is only supported on serial meshes
  distribution = serial
[]

[Variables/nodal]
  family = LAGRANGE
  order = FIRST
  initial_from_file_var = nodal
  initial_from_file_timestep = 10
[]

[AuxVariables/elemental]
  family = MONOMIAL
  order = CONSTANT
  initial_from_file_var = elemental
  initial_from_file_timestep = 10
[]
```

!---

## Checkpoints

Advanced restart and recovery in MOOSE require checkpoint files

Checkpoints are automatically enabled by default and are output every 1 hour of wall time (customizable interval), but can be disabled with:
```text
[Outputs]
  wall_time_checkpoint = false
[]
```

Checkpoints can be output at every time step with the following shortcut syntax:

```text
[Outputs]
  checkpoint = true
[]
```

!---

For more control over the checkpoint system, create a sub-block in the input file that will allow you
to change the file format, suffix, frequency of output, the number of checkpoint files to keep, etc.

- Set `num_files` to at least 2 to minimize the chance of ending up with a corrupt restart file

  !listing outputs/checkpoint/checkpoint_interval.i block=Outputs

!---

## Advanced Restart

This method is best suited for situations when the mesh from the previous simulation and the current
simulation match and the variables and stateful data should be loaded from the pervious simulation.

- Support for modifying some variables is supported such as `dt` and `time_step`. By default, MOOSE
  will automatically use the last values found in the checkpoint files
- Only N to N restarts are supported using this method

```text
[Mesh]
  # Serial number should match corresponding Executioner parameter
  file = out_cp/0010-mesh.cpr
  # This method of restart is only supported on serial meshes
  distribution = serial
[]

[Problem]
  # Note that the suffix is left off in the parameter below.
  restart_file_base = out_cp/LATEST  # You may also use a specific number here
[]
```

!---

## Reloading Data

It is possible to load and project data onto a different mesh from a solution file usually as an
initial condition in a new simulation.

MOOSE supports this through the use of a SolutionUserObject

!---

## Recover

A simulation that has terminated due to a fault can be recovered simply by using the `--recover`
command-line flag, but it +requires a checkpoint file+.

```bash
./frog-opt -i input.i --recover
```

!---

## Multiapp Restart

When running a multiapp simulation you do +not+ need to enable checkpoint output in each sub app
input file. The parent app stores the restart data for all sub apps in its file.
