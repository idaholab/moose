# ControlData

ControlData is produced and consumed by [ControlLogic](syntax/ControlLogic/index.md) objects.
The producer usually declares it and sets it value, while the consumer uses the value to make control
decisions during the simulation.

ControlData has an internal container, `ControlDataValue`, which can hold standard C++ types. It is
often used with boolean or `Real` values. It also keeps track of its value at the previous time step.

ControlData is naturally restored during simulation restart and recover.
