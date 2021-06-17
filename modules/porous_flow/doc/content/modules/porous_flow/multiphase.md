# Multiphase flow

Presently, PorousFlow provides several `Materials` to model two-phase flow. However,
all of the PorousFlow `Kernels` are designed to be used with an arbitrary number of
fluid phases, so that additional phases can be included in PorousFlow by simply creating
new `Materials` that provide properties for each phase.

Two-phase flow can be modelled using either formulations where porepressure of
each phase are the nonlinear variables, or where porepressure and saturation of
one phase are solved. These `Materials` calculate the pressure and saturation of each
fluid phases, as well as all derivatives with respect to the PorousFlow variables.

- Two-phase flow with porepressure formulation [PorousFlow2PhasePP](/PorousFlow2PhasePP.md), and its hysteretic cousin [PorousFlow2PhaseHysPP](/PorousFlow2PhaseHysPP.md)
- Two-phase flow with porepressure - saturation formulation [PorousFlow2PhasePS](/PorousFlow2PhasePS.md), and its hysteretic cousin [PorousFlow2PhaseHysPS](/PorousFlow2PhaseHysPS.md)
