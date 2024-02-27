
# Enthalpy mixing model Verification

## Test Description

&nbsp;

This verification problem is the same used in [!cite](CTF-Verification). This case presents a problem where the effects of turbulent mixing are clearly discernible and quantifible. Turbulence causes both momentum and enthalpy mixing through the terms:

\begin{equation}
Drag_{ij} = -C_{T}\sum_{j} w'_{ij}\Delta U_{ij }
\end{equation}

\begin{equation}
h'_{ij} = \sum_{j} w'_{ij}\Delta h_{ij}
\end{equation}

Because the model for turbulent mixing is gradient-driven based in $\Delta h, \Delta U$, in order to observe the effects of turbulence, it is necessary to make a gradient in either energy or momentum. It is easier to focus on the energy equation and deactivate the density calculation. The problem geometry consists of two identical channels connected by a gap and is seen in [enthalpy].

!media figures/enthalpy.png
    style=width:30%;margin-bottom:2%;margin:auto;
    id=enthalpy
    caption=Enthalpy mixing model verification problem geometry

To test the turbulent mixing model, the temperature of one channel is raised by 10 degrees Celsius. Turbulent enthalpy mixing will transfer heat from the hot channel to the cold channel. The solution given by the code is then compared to the analytical solution:

\begin{equation}
h_1 = \frac{(h_{1,in} + h_{2,in})}{2} -  \frac{1}{2}(h_{2,in} - h_{1,in})\exp(-\frac{2 \frac{dw_{12}'}{dz}}{\dot{m}} z)
\end{equation}

\begin{equation}
h_2 = \frac{(h_{1,in} + h_{2,in})}{2} + \frac{1}{2}(h_{2,in} - h_{1,in})\exp(-\frac{2 \frac{dw_{12}'}{dz}}{\dot{m}} z)
\end{equation}

## Results

The analytical solution is compared with the code results in [enthalpy-ver]. The code results are in good agreement with the analytical solution.

!media figures/enthalpy-ver.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=enthalpy-ver
    caption=Enthalpy distribution in the axial direction

## Input file

To run the enthalpy mixing model verification problem use the following input file:

```bash
mass_flux_in = 3500 # kg /sec m2
P_out = 155e+5 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 2
    ny = 1
    n_cells = 100
    pitch = 0.0126
    rod_diameter = 0.00950
    gap = 0.00095
    heated_length = 10.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[Functions]
  [T_fn]
    type = ParsedFunction
    value = if(x>0.0,483.10,473.10)
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
  CT = 2.0
  P_tol = 1e-6
  T_tol = 1e-6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
[]

[ICs]
  [S_ic]
    type = ConstantIC
    variable = S
    value = 8.78778158e-05
  []

  [T_ic]
    type = FunctionIC
    variable = T
    function = T_fn
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
