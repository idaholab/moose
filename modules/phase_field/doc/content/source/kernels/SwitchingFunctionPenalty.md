# SwitchingFunctionPenalty

!syntax description /Kernels/SwitchingFunctionPenalty

The penalty is added in the form of a free energy term

!equation
F_p=\gamma k(\vec \eta)^2,

where $\gamma$ is the penalty prefactor (`penalty`) and $k = 1-\sum_i\eta_i$ is
the constraint. The constraint is enforced approximately to a tolerance of
$\frac1\gamma$ (depending on the shape and units of the free energy).

Also see: [Multiphase models](/MultiPhase/WBM.md)

!syntax parameters /Kernels/SwitchingFunctionPenalty

!syntax inputs /Kernels/SwitchingFunctionPenalty

!syntax children /Kernels/SwitchingFunctionPenalty
