# Steady-state mixing model Validation

The PSBT 5x5 benchmark is an international benchmark developed by the Organisation for Economic Co-operation and Development (OECD), the Nuclear Regulatory Commission (NRC), and the Nuclear Power Engineering Center(NUPEC) [!cite](PSBT1). In this work we utilize the steady-state mixing test, detailed in volume III of the PSBT benchmark [!cite](PSBT3). The purpose of this test is to validate the mixing models of the participating codes. The participating codes predict the fluid temperature distribution at the exit of the heated section of a rod bundle assembly and compare it with the experimental values provided by the benchmark. Here we will present as an example case 01-5237, as well as the average error for all the cases run [!cite](kyriakopoulos2022development). The rod bundle geometry description is presented in Table below.

| PSBT rod bundle specifications |
| |

| Item | Value |
| - | - |
| Rods array | $5\times5$ |
| Number of heated rods | $25$ |
| Heated rod outer diameter (mm) | $9.50$ |
| Pitch (mm) | $12.60$ |
| Axial heated length (mm) | $3658$ |
| Flow channel inner width (mm) | $64.9$  |
| Axial power shape | Uniform  |
| Number of mixing vaned (MV) spacers | $7$  |
| Number of non mixing vaned (NMV) spacers | $2$  |
| Number of simple spacers (SS) | $8$  |
| MV spacer location (mm) | $457,914,1372,1829,2286,2743,3200$ |
| NMV spacer location (mm) | $0,3658$  |
| Simple spacer location (mm) | $229,686,1143,1600,2057,2515,2972,3429$ |

The rod bundle has a radial power profile in which the right side of the assembly is under-heated. The radial power profile is shown in [fig:Power]. The rods on the left side transfer 100% of available rod power to the fluid, while the rods on the right transfer 25%. This causes an uneven temperature distribution at the exit of the assembly.

!media figures/Radial_Power.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:Power
    caption=Radial pin power profile

Note that the turbulent mixing parameter used for all the cases was: $β = 0.08$, which differs significantly from the default value of $β = 0.006$. The reason behind the need to adjust $β$ to a much higher value, has to do with the geometry of the PSBT facility. Note that there is a preferential mixing direction of the experimental results in the diagonal direction (towards one corner of the assembly),
exhibited in [fig:profile], while the code results for both values of $β$ are symmetric as expected. The experimental results exhibit a non symmetric distribution that we cannot capture with a constant value of beta. There is a temperature gradient towards the corner due to an additional mixing effect, which may reduce the exit temperature differences between the two regions (hot/cold, left/right) and finally increase the optimum $β$ in comparison with the case of no temperature gradient in each region.

!media figures/01-5237.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:profile
    caption=Exit temperature distribution for case 01-5237

The reason for the additional mixing effect is thought to be the special mixing vanes that the NUPEC facility incorporates in its design. Specifically, the temperature gradient appearing in the experimental data was attributed to the thermal mixing in the diagonal direction of the test bundle, which may be caused by the alignment of mixing vanes mounted in the spacer grids [!cite](hwang2012accuracy). This is the physical reason behind the need to use an increased value of $\beta = 0.08$.

This illustrates the fact that modeling parameters like $\beta$ should ideally be calibrated for specific geometries and in no way can they be applied generally without proper justification. Nevertheless our results show that, provided we choose the optimum parameters adjusted for the specific geometries, we can accurately predict the exit temperatures. Figure [fig:MixingError] presents the cumulative mean absolute error in the exit temperature in comparison with other subchannel codes [!cite](PSBT3). We note that for the temperature mixing test, our code performs adequately in comparison to the other codes. Our mean absolute error is calculated to be $Error = 2.176$ which places us in 5th place out of the nine codes.

!media figures/psbt-results.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:MixingError
    caption=Mean absolute error in predicted exit temperature

## Input file for case: 01-5237

```bash
# M. Avramova et. all 2012,
# OECD/NRC Benchmark Based on NUPEC PWR
# Sub-channel and Bundle Tests (PSBT). Volume III: Departure from Nucleate Boiling
T_in = 502.35
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = ${fparse 1e+6 * 16.95 / 3600.}
P_out = 14.72e6 # Pa
[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 6
    ny = 6
    n_cells = 50
    pitch = 0.0126
    pin_diameter = 0.00950
    gap = 0.00095
    heated_length = 3.658
    spacer_z = '0.0 0.229 0.457 0.686 0.914 1.143 1.372 1.600 1.829 2.057 2.286 2.515 2.743 2.972 3.200 3.429'
    spacer_k = '0.7 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4'
  []

  [fuel_pins]
    type = QuadPinMeshGenerator
    input = sub_channel
    nx = 6
    ny = 6
    n_cells = 50
    pitch = 0.0126
    heated_length = 3.658
  []
[]

[AuxVariables]
  [mdot]
    block = sub_channel
  []
  [SumWij]
    block = sub_channel
  []
  [P]
    block = sub_channel
  []
  [DP]
    block = sub_channel
  []
  [h]
    block = sub_channel
  []
  [T]
    block = sub_channel
  []
  [Tpin]
    block = fuel_pins
  []
  [rho]
    block = sub_channel
  []
  [mu]
    block = sub_channel
  []
  [S]
    block = sub_channel
  []
  [w_perim]
    block = sub_channel
  []
  [q_prime]
    block = fuel_pins
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
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
[]

[ICs]
  [S_IC]
    type = QuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = QuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = QuadPowerIC
    variable = q_prime
    power = 3.23e6 # W
    filename = "power_profile.txt"
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
    height = 3.658
  []
  [mdot_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_Out.txt"
    height = 3.658
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
