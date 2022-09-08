# DisplacedSystem

`DisplacedSystem` objects mostly use global vector and matrix data from the
undisplaced [nonlinear](NonlinearSystemBase.md) and
[auxiliary](AuxiliarySystem.md) systems. However, it holds distinct variable
warehouses such that residual objects can use variables evaluated using finite
element shape functions reinitialized with displaced mesh elements.
