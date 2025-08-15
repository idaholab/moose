# ControllableInputTimes

!syntax description /Times/DynamicInputTimes

This object should be used to specify a time sequence dynamically updating during the simulation.


`ControllableInputTimes` supports two inputs: a controllable real-valued `nexttime` and a noncontrollable time sequences input `times`. If both are provided, they’re merged and kept in sorted order to form a single schedule.

In the example, `RealFunctionControl` computes the next event time during the run and updates `ControllableInputTimes` accordingly. You can predefine fixed event times with `times`, and adjust `nexttime` on the fly via the control block.

!listing test/tests/times/external_times.i

!syntax parameters /Times/DynamicInputTimes

!syntax inputs /Times/DynamicInputTimes

!syntax children /Times/DynamicInputTimes
