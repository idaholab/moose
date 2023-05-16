# ExodusTimeSequenceStepper

!syntax description /Executioner/TimeSteppers/ExodusTimeSequenceStepper

The Exodus file is read by the first process (rank 0), and the time step sequence is then broadcast to all other processes.

If the solve fails to converge during a time step, the behavior of the `ExodusTimeSequenceStepper` is the same as the [TimeSequenceStepper.md]. The time step will be cut then the time stepper will attempt to return to the original sequence.

## Example input file

In this example, the time stepper extracts the time sequence from an exodus output file. This exodus file may not necessarily have been generated using the same input file. The starting and end time of the simulation may still be set independently.

!listing test/tests/time_steppers/timesequence_stepper/exodustimesequence.i block=Executioner

!syntax parameters /Executioner/TimeSteppers/ExodusTimeSequenceStepper

!syntax inputs /Executioner/TimeSteppers/ExodusTimeSequenceStepper

!syntax children /Executioner/TimeSteppers/ExodusTimeSequenceStepper
