# InputTimes

!syntax description /Times/InputTimes

This object should be used to specify a few times. A [CSVFileTimes.md] should be preferred
if there are many times.

Dynamic time sequence is also supported, which means you can add/update the time sequence during simulation time via control. As shown in the example, `RealFunctionControl` will change the time sequence in `InputTimes` according to a function and compute the next time to hit during the simulation.

!listing test/tests/times/external_times.i


!syntax parameters /Times/InputTimes

!syntax inputs /Times/InputTimes

!syntax children /Times/InputTimes
