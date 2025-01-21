# Restart, recover and initialization hands-on

!---

## Initialization from Exodus file

!listing step13_restart/initialization_from_exodus.i block=Mesh Variables AuxVariables

Notes:

- only variables with `initial_from_file_var` set will be initialized
- reading exodus files is a replicated operation, meaning the file is loaded by every process in parallel. Use Nemesis files in parallel to reduce memory costs

!---

Hands-on:

```
cd step13_restart
# Generate the initialization file
../executable/shield_multiphysics-opt -i base_calc.i
# Use the exodus file to initialize some variables
../executable/shield_multiphysics-opt -i initialization_from_exodus.i
```

!---

## Restart from a Checkpoint

!listing step13_restart/restart_from_checkpoint.i block=Mesh Problem

Notes:

- initial conditions specified in the input override any restarted fields.
  This must be allowed in the `Problem` block to avoid unintentional behavior.

!---

Hands-on:

```
cd step13_restart
# Generate the initialization file
../executable/shield_multiphysics-opt -i base_calc.i
# Use the checkpoint file to restart the simulation from where it finished
../executable/shield_multiphysics-opt -i restart_from_checkpoint.i
```

!---

## Recover from Checkpoint

Recover lets us start from the last checkpoint before the simulation
stopped.

```
../executable/shield_multiphysics-opt -i base_calc.i
# Use Ctrl+C / Cmd+C to cancel the calculation
# Recover from the last valid checkpoint
../executable/shield_multiphysics-opt -i base_calc.i --recover
```
