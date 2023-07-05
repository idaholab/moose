# ExodusTimeSequenceStepper

This time stepper derives from [TimeSequenceStepperBase.md] and provides the
sequence of time values from an Exodus file.

The Exodus file is read by the first process (rank 0), and the time step sequence is then broadcast to all other processes.

See [TimeSequenceStepperBase.md#failed_solves] for information on the behavior
of this time stepper for failed time steps.

## Example input file

In this example, the time stepper extracts the time sequence from an exodus output file. This exodus file may not necessarily have been generated using the same input file. The starting and end time of the simulation may still be set independently.

!listing test/tests/time_steppers/timesequence_stepper/exodustimesequence.i block=Executioner

!syntax parameters /Executioner/TimeSteppers/ExodusTimeSequenceStepper

!syntax inputs /Executioner/TimeSteppers/ExodusTimeSequenceStepper

!syntax children /Executioner/TimeSteppers/ExodusTimeSequenceStepper
