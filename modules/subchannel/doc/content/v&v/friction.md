
# Friction model Verification

## Test Description

&nbsp;

This verification problem is the same used in [!cite](CTF-Verification). This case presents a problem where the effects of friction are clearly discernible and quantifible. Momentum transfer in the single-phase case is driven by a lateral pressure gradient and turbulence. By deactivating turbulence in the test model case, momentum transfer can only be the result of lateral pressure imbalance; which for a model with no form losses (spacer grids), can only be driven by unequal frictional losses. Friction loss depends on the hydraulic diameter, so it makes sense to devise a two-channel problem, with channels that have an unequal flow area. The problem geometry is shown in [fig-friction].

!media figures/friction.png
    style=width:30%;margin-bottom:2%;margin:auto;
    id=fig-friction
    caption=Friction model verification problem geometry

Channel-2 has a hydraulic diameter that is twice the size of the Channel-1 hydraulic diameter. The length of the model is set to 10 m to allow the flow to completely redistribute within the solution space. The different frictional pressure drops create a lateral pressure gradient that drives flow from the higher resistance channel to the lower resistance channel. Moving up the channels, velocity grows larger in the low-resistance channel, which increases frictional pressure drop in that channel. Simultaneously, velocity decreases in the high-resistance channel, which decreases frictional pressure drop. This continues until the frictional pressure drop is the same in both channels, at which point crossflow ceases. At this point, the channels are said to be in mechanical equilibrium. An analytical solution can be derived for this point of mechanical equilibrium:

\begin{equation}
\dot{m}_{in} = \dot{m}_2 \bigg( 1 + \bigg(  \frac{D_{h,2}}{D_{h,1}}\bigg)^{\frac{C_2 - 1}{C_2 + 2}} \frac{A_1}{A_2}\bigg).
\end{equation}

## Results

&nbsp;

The analytical prediction is compared with the code results in [friction-ver]. The code results converge to the analytical solution at the mechanical equilibrium.

!media figures/friction-ver.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=friction-ver
    caption=Relative mass flow distribution in the axial direction

## Input file

To run the friction model verification problem use the following input file:

```bash
T_in = 473.15 # K
mass_flux_in = 3500 # kg /sec m2
P_out = 155e+5 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 1
    ny = 2
    n_cells = 100
    pitch = 0.0126
    pin_diameter = 0.00950
    gap = 0.00095
    heated_length = 10.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[Functions]
  [S_fn]
    type = ParsedFunction
    value = if(y>0.0,0.002,0.001)
  []
[]

[Modules]
  [FluidProperties]
    [water]
      type = Water97FluidProperties
    []
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  beta = 0.006
  CT = 0.0
  P_tol = 1e-6
  T_tol = 1e-6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
[]

[ICs]
  [S_ic]
    type = FunctionIC
    variable = S
    function = S_fn
  []

  [w_perim_IC]
    type = ConstantIC
    variable = w_perim
    value = 0.34188034
  []

  [q_prime_IC]
    type = ConstantIC
    variable = q_prime
    value = 0.0
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0
  []

  [DP_ic]
    type = ConstantIC
    variable = DP
    value = 0.0
  []

  [Viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = water
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = water
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = water
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
  []
  [mdot_in_bc]
    type = MassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out.txt"
    height = 10.0
  []
  [mdot_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_Out.txt"
    height = 10.0
  []
  [mdot_In_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_In.txt"
    height = 0.0
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]

```
