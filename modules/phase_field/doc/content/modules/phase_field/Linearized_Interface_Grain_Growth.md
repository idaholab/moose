# Linearized Interface Grain Growth Model

## Introduction

!media /phase_field/linearized_interface_function.png style=width:40%;margin-left:1%;float:right;
       caption=Figure 1: Example of order parmeter values ($\phi_1$, $\phi_2$) and the linearized interface transformed variables ($\psi_1$, $\psi_2$),

In a phase field grain growth model, like that developed by [!cite](moelans_quantitative_2008) and [implemented in MOOSE](Grain_Growth_Model.md), the order parameters that represent the grains tend to follow a $\tanh$ profile across the interface. A fine mesh is required to resolve this highly nonlinear profile, such that at least three elements are typically required across the grain boundaries. This fine mesh required at the grain boundaries makes the computational cost of grain growth simulations high.

The cost of grain growth simulations can be reduced by using a change of variables to linearize the interface profile, sometimes referred to as nonlinear preconditioning and first suggested by [!cite](glasner2001nonlinear). This change in variables is defined by

\begin{equation}
    \phi_i = \frac{1}{2} \left[ 1 + \tanh\left( \frac{\psi_i}{\sqrt{2}} \right) \right],
\end{equation}

where $\phi_i$ is the original order parameter and $\psi_i$ is the new variable. While $\phi_i$ follows a $\tanh$ profile across the interface, the new variable $\psi_i$ is linear across the interface, as illustrated in Figure 1. Due to the linear profile across the interface, a single linear element can resolve the profile without error. This significantly decreases the required resolution, and thus the computational cost.

## MOOSE Implementation

Linearized interface has been implemented in the multiphase field grain growth model in MOOSE. The basic model is identical to the [standard phase field model in MOOSE](/Grain_Growth_Model.md), but it solves for the transformed linearized interface variables rather than the standard order parameters. The order parameters are created as AuxVariables, allowing them to be visualized. The grain boundary can be visualized using an AuxVariable equal to $\sum_i \phi_i^2$, calculated using [BndsCalcAux](/BndsCalcAux.md) in the usal manner.

The MOOSE objects used in the linearized interface grain growth model are listed, below:

+Materials+

- [LinearizedInterfaceFunction](/LinearizedInterfaceFunction.md) - Defines the order parameters $\phi_i$ from the $\psi_i$ linear interface variable values at the quadrature points. It uses the [ExpressionBuilder](/ExpressionBuilder.md) so it also calculates the analytical derivatives.

+Kernels+

- [ChangedVariableTimeDerivative](/ChangedVariableTimeDerivative.md) - Version of the [TimeDerivative](/TimeDerivative.md) kernel after a change of variable.
- [ACGrGrPolyLinearizedInterface](/ACGrGrPolyLinearizedInterface.md) - Linearized interface version of the [ACGrGrPoly](/ACGrGrPoly.md) kernel.
- [ACInterfaceChangedVariable](/ACInterfaceChangedVariable.md) - Version of the [ACInterface](/ACInterface.md) kernel after a change of variable.

+AuxKernels+

- [LinearizedInterfaceAux](/LinearizedInterfaceAux.md) - Defines AuxVariable values of the order parameters $\phi_i$ from the $\psi_i$ nonlinear variable values at the nodes for the linearized interface grain growth model.

## Bounded Solve

When solving the linearized interface version of the grain growth model, [!cite](gong2018fast) and [!cite](chadwick2021development) found that the solve is unstable unless bounds are placed on the possible values for the transformed variables. Therefore, upper and lower bounds must be defined in the `[Bounds]` block for each variable using [ConstantBoundsAux](/ConstantBoundsAux.md). A typical bounds range is $\pm5$. For example:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i block=Bounds

Note that in order for these bounds to have an effect, the user has to specify the
PETSc options `-snes_type vinewtonssls` or `-snes_type vinewtonrsls`. A warning will be generated if neither options are specified. The PETSc manual pages for the `vinewtonssls` algorithm
can be found
[here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONSSLS.html)
while the manual page for `vinewtonrsls` can be found
[here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONRSLS.html#SNESVINEWTONRSLS).

These solve options are demonstrated here:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i block=Executioner

## Simplified MOOSE syntax

As with the standard grain growth model in MOOSE, an Action has been created to simplify the input file syntax needed to define a nonlinear preconditioning grain growth model. The [GrainGrowthLinearizedInterfaceAction](/GrainGrowthLinearizedInterfaceAction.md) allows the user to specify the number of order parameters to be used in the model and it automatically generates the

- Linearized interface variables $\psi_i$
- AuxVariables and corresponding Auxkernels defining the order parameters $\phi_i$
- All of the kernels
- Material properties defining the order parameters $\phi_i$
- Upper and lower bounds required for the solve
- Auxvariable and kernel defining the grain boundary for visualization

The custom syntax for the linearized interface grain growth model is nearly identical to that for the standard grain growth model, making it simple to switch back and forth between models.

## linearized interface Initial Conditions

Initial conditions have been created to transform the IC values for the standard grain growth model to the linearized interface model.

- [SmoothCircleICLinearizedInterface](/SmoothCircleICLinearizedInterface.md) - Takes the output from the [SmoothCircleIC](/SmoothCircleIC.md) and performs the change in variable. Used to model a shrinking circular grain.
- [PolycrystalColoringICAction](/PolycrystalColoringICAction.md) - This custom syntax is used for all polycrystal ICs for both the standard grain growth model and with nolinear preconditioning. The parameter `nonlinear_preconditioning = true` uses the [PolycrystalColoringICLinearizedInterface](/PolycrystalColoringICLinearizedInterface.md) for each variable.

## Usage with GrainTracker

[GrainTracker](/GrainTracker.md), which allows variables to represent multiple grains and remap them to other variables to avoid coalescence, also works with the linearized interface grain growth model. Several changes are needed in its use:

1. The input parameter `bound_value` should be set to the value used in the bounded solve. This will switch the variable value when a grain is remapped to -`bound_value`, erasing the grain from the original variable. This is the same parameter used in [GrainGrowthLinearizedInterfaceAction](/GrainGrowthLinearizedInterfaceAction.md) and in the linearized interface ICs, so that it can be set for all of the objects from the [GlobalParams block](/GlobalParamsAction.md).
2. The `threshold` parameter must be changed to be slightly above the value of the lower bound (-`bound_value`). It is recommended to use -`bound_value` + 2*`bound_value`/10.

The modified syntax is illustrated here:

!listing modules/phase_field/test/tests/grain_tracker_test/grain_tracker_remapping_linearized_interface_test.i block=GlobalParams

!listing modules/phase_field/test/tests/grain_tracker_test/grain_tracker_remapping_linearized_interface_test.i block=UserObjects/grain_tracker

## Example Input Files

An example input file for a polycrystal simulation using the linearized interface grain growth model is available:

!listing modules/phase_field/examples/grain_growth/grain_growth_linearized_interface.i

The linearized interface tests also serve as good examples for how to use the capability.

The evolution of a shrinking circular grain is illustrated here:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/grain_growth_linearized_interface.i

The same problem but using [GrainGrowthLinearizedInterfaceAction](/GrainGrowthLinearizedInterfaceAction.md) is illustrated here:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/linearized_interface_action.i

The evolution of a five-grain polycrystal is illustrated here:

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/voronoi_linearized_interface.i
