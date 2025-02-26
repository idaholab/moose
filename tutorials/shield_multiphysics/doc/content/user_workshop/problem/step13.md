# Step 13: Restart, recover and initialization hands-on id=step13

!---

## Step 13: Initialization from Exodus file

!style fontsize=75%
!listing step13_restart/step13b_initialization_from_exodus.i
         diff=step13_restart/step13a_base_calc.i
         block=Mesh Variables Executioner
         max-height=300px

!alert note
- only variables with `initial_from_file_var` set will be initialized
- reading exodus files is a replicated operation, meaning the file is loaded by every process in parallel. Use Nemesis files in parallel to reduce memory costs

!---

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step13_restart
# Generate the initialization file
moose-opt -i step13a_base_calc.i
# Use the exodus file to initialize some variables
moose-opt -i step13b_initialization_from_exodus.i
```

!---

## Step 13: Restart from a Checkpoint

!style! fontsize=75%

!listing step13_restart/step13a_base_calc.i block=Outputs

!listing step13_restart/step13c_restart_from_checkpoint.i
         diff=step13_restart/step13a_base_calc.i
         block=Mesh Problem
         max-height=200px

!style-end!

!alert note
initial conditions specified in the input override any restarted fields.
This must be allowed in the `Problem` block to avoid unintentional behavior.

!---

```bash
moose-opt -i step13a_base_calc.i
moose-opt -i step13c_restart_from_checkpoint.i
```

!---

## Step 13: Recover from Checkpoint

Recover lets us start from the last checkpoint before the simulation
stopped.

```bash
moose-opt -i step13a_base_calc.i
# Use Ctrl+C / Cmd+C to cancel the calculation
# Recover from the last valid checkpoint
moose-opt -i step13a_base_calc.i --recover
```
