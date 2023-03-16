# Restart and Recover

## Definitions

- +Restart+: Running a simulation that uses data from a previous simulation. Data in this context is very broad, it can mean spatial field data, non-spatial variables or postprocessors, or stateful object data. Usually the previous and new simulations use different input files.
- +Recover+: Resuming an existing simulation either due to a fault or other premature termination.
- +Solution File+: A mesh format containing field data in addition to the mesh (i.e. a normal output file).
- +Checkpoint+: A snapshot of the simulation data including all meshes, solutions, and stateful object data. Typically one checkpoint is stored in several different files.
- +N to N+: In a restart context, this means the number of processors for the previous and current simulations must match.
- +N to M+: In a restart context, different numbers of processors may be used for the previous and current simulations.

## Variable Initialization

- This method is best suited for restarting a simulation when the mesh in the previous simulation exactly matches the mesh in the current simulation and only initial conditions need to be set for one more variables.
- This method requires only a valid Solution File.
- MOOSE supports N to M restart when using this method.

```puppet
[Mesh]
  #MOOSE supports reading field data from ExodusII, XDA/XDR, and mesh checkpoint files (.e, .xda, .xdr, .cp)
  file = previous.e
  #This method of restart is only supported on serial meshes
  distribution = serial
[]

[Variables]
  [./nodal]
     family = LAGRANGE
     order = FIRST
     initial_from_file_var = nodal
     initial_from_file_timestep = 10
  [../]
[]

[AuxVariables]
  [./elemental]
     family = MONOMIAL
     order = CONSTANT
     initial_from_file_var = elemental
     initial_from_file_timestep = 10
  [../]
[]
```

## Enabling Checkpoints

- Advanced restart in MOOSE requires checkpoint files.
- To enable constant checkpoint writing using the default options (every time step, and keep last two) in your simulation simply add the following flag to your input file:

```puppet
[Outputs]
  checkpoint = true
[]
```

*If you need more control over the checkpoint system, you can create a subblock in the input file that will allow you to change the file format, suffix, frequency of output, the number of checkpoint files to keep, etc.*

For a complete list see the Doxygen page for Checkpoint. * You should always set `num_files` to at least 2 to minimize the chance of ending up with a corrupt restart file.

```puppet
[Outputs]
  [./my_checkpoint]
    type = Checkpoint
    num_files = 4
    interval = 5
  [../]
[]
```

MOOSE also automatically creates a checkpoint object in the background that can manually write out a checkpoint file at any time in case of emergency, i.e. a long test that must be aborted due to external circumstances. To do this, find the process ID by running `ps` in another terminal window, and searching for your currently running MOOSE instance. Once you have located this PID, enter `kill -s USR1 <yourPIDhere>`. On the next time step, MOOSE will output its current progress into a checkpoint file that can be used later to restart the test from the same position.

Note that while this command is called `kill`, it does not actually terminate the MOOSE process if used with this syntax, it will merely trigger the MOOSE instance to write out to a checkpoint.

## Advanced Restart

- This method is best suited for situations when the mesh from the previous simulation and the current simulation match but all variables should be reloaded and all stateful data should be restored.
- Support for modifying some variables is supported such as dt and time_step. By default, MOOSE will automatically use the last values found in the checkpoint files.
- Only N to N restarts are supported using this method.

```puppet
[Mesh]
  #Serial number should match corresponding Executioner parameter
  file = out_cp/0010_mesh.cpr
  #This method of restart is only supported on serial meshes
  distribution = serial
[]

[Problem]
  #Note that the suffix is left off in the parameter below.
  restart_file_base = out_cp/LATEST  # You may also use a specific number here
[]
```

## Reloading Data

- It is possible to load and project data onto a different mesh from a solution file usually as an initial condition in a new simulation.
- MOOSE fully supports this through the use of [SolutionUserObject](SolutionUserObject.md).

## Recover

- A simulation that has terminated due to a fault can be recovered simply by using th `--recover` CLI flag.
- Requires checkpoint files.

## MultiApp Restart

When running a multiapp simulation you do +not+ need to enable checkpoint output in each sub app input file. The master app stores the restart data for all sub apps in its file.
More information about MultiApp restart/recover can be found at [MultiApps](syntax/MultiApps/index.md).

## Time Control on Restart

`start_time` can be continued on from the previous simulation, or can be overridden on restart. If this parameter is omitted from your input file
the default will be to continue from the previous simulation. If you supply the parameter in your input file, the new simulation will begin from
the supplied time.