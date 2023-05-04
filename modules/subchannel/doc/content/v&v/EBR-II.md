
# EBR-II, SHRT-17 Validation

## Test Description

&nbsp;

 On April 3, 1986, two tests were carried out to demonstrate the effectiveness of passive feedback in the EBR-II reactor. Both tests began from full reactor power and were initiated when both the primary coolant pumps and the intermediate loop pump were simultaneously tripped, to simulate a loss-of-flow accident. SHRT-17 was a protected loss-of-flow (LOF) test and SHRT-45R was an unprotected loss-of-flow (ULOF) test. At the beginning of test SHRT-17, the primary pumps were tripped at the same time as a full control rod insertion. SHRT-45R, was similar to SHRT-17 except that during this test the plant protection system (PPS) was disabled, to prevent it from initiating a control rod scram. In the second test, reactor power decreased due to reactivity feedback effects.

 The EBR-II reactor vessel grid plenum sub-assembly accommodated 637 hexagonal sub-assemblies, which were installed in one of three regions: a central core, inner blanket, or outer blanket. The central core comprised the 61 sub-assemblies in the first five rows. Pronghorn's subchannel code (Pronghorn-SC) is compared with data measured in the XX09 instrumented sub-assembly. More particularly, the code calculations are compared against temperature profile measurements in various axial elevations and the transient temperature evolution of the peak temperature in the central subchannel.

### Plant Overview

Argonne National Laboratory’s (ANL) Experimental Breeder Reactor II (EBR-II) was a liquid metal reactor with a sodium-bonded metallic fuel core. EBR-II was rated for a thermal power of 62.5 MW with an electric output of approximately 20 MW. A schematic of the reactor and the primary sodium flow paths are shown in [fig:schematic]. All major primary system components were submerged in the primary tank, which contained approximately $340 m^3$ of liquid sodium at $371^o C$. Two primary pumps inside this pool provided sodium to the two inlet plena of the core. Sub-assemblies in the inner core received sodium from the high-pressure inlet plenum, accounting for approximately $85\%$ of the total primary flow. The blanket and reflector sub-assemblies in the outer blanket region received sodium from the low-pressure inlet plenum. Hot sodium exited the sub-assemblies into a common upper plenum, where it mixed before passing into the intermediate heat exchanger (IHX).

!media figures/EBR-II_primary_tank.png
    style=width:50%;margin-bottom:2%;margin:auto;
    id=fig:schematic
    caption=Schematic of the EBR-II reactor [!cite](osti_801571)

The reactor-vessel grid-plenum sub-assembly accommodated 637 hexagonal sub-assemblies. The sub-assemblies were divided into three regions: core, inner blanket (IB) and outer blanket (OB). The central core comprised the 61 sub-assemblies in the first five rows. Two positions in row 3 contained safety-rod sub-assemblies and eight positions in Row 5 contained control-rod sub-assemblies. Two positions in Row 5 contained the instrumented sub-assemblies (INSAT) XX09 and XX10, and one position in Row 5 contained the in-core instrument test facility (INCOT) XY16. The remainder of the central core region contained driver fuel or experimental-irradiation sub-assemblies. EBR-II was heavily instrumented to measure mass flow rates, temperatures, and pressures, throughout the system.

### Instrumented sub-assemblies

The SHRT-17 test is a protected LOF test. This test was initiated by a trip of the primary and intermediate pumps under the rated-power of 57.3MW. The reactor was scrammed at the same time as the pump trips. As flow dissipated in the primary system after the pump trips, cooling of the core transitions from forced to natural circulation, while temperature and flow rate converge to an equilibrium state.

The SHRT-45R test is an unprotected LOF test. Similarly to SHRT-17, this test was initiated by a trip of the primary and intermediate pumps under the rated-power of 60.0 MW, but without scram. In the SHRT-45R experiment, the auxiliary electromagnetic pump (EMP) was kept operational throughout the test duration. As a result, the cooling situation is not fully natural convection, but since the EMP flow rate is very low, it is also not fully forced circulation. Because of the operation of the EMP in the SHRT-45R experiment, the coolant mass flow rate converges to about two times that in the SHRT-17 experiment.

Both tests run for $900$ seconds.

This work utilizes data measured by the instrumented sub-assembly XX09. XX09 was a fueled sub-assembly specifically designed with a variety of instrumentation to provide data for benchmark validation purposes. The standard-type fueled sub-assembly contains 91 fuel pins, whereas in XX09 the outer row of fuel pins was removed and a guide thimble was inserted instead, as shown in [fig:XX09], [fig:XX09_3].

!media figures/XX09.png
    style=width:50%;margin-bottom:2%;margin:auto;
    id=fig:XX09
    caption=XX09 Instrumented sub-assembly axial-section [!cite](summer2012benchmark)

!media figures/XX09_3.png
    style=width:50%;margin-bottom:2%;margin:auto;
    id=fig:XX09_3
    caption=XX09 Instrumented sub-assembly cross-section [!cite](summer2012benchmark)

### Boundary conditions

The values for the inlet mass flow rate, power, and inlet coolant temperature are specified in the EBR-II SHRT benchmark [!cite](summer2012benchmark). The value of outlet pressure has been approximated based on the operation conditions of the EBR-II before the transients. These values represent the steady state conditions leading to the transients.

| Experimental steady-state parameters |
| |

| Experiment Parameter (Unit) | SHRT-17 | SHRT-45R |
| - | - | - |
| Inlet Mass flow rate of XX09 (kg/s) | 2.45 | 2.427 |
| Power of XX09 (kW) | 486.2 | 379.8 |
| Inlet coolant temperature (K) | 624.7 | 616.4 |
| Outlet Pressure (kPa) | 200.0 | 200.0 |

During the SHRT transients, the mass flow rate and power vary. The normalized power and mass flow rate during the transients have been adapted from [!cite](mochizuki2014benchmark) , [!cite](mochizuki2018benchmark) and are presented in [fig:Norm_TR]. This information is used as input for the subchannel code transient calculations. The work in the cited sources utilizes a NETFLOW++ [!cite](mochizuki2010development) simulation to inform a COBRA-IV-I [!cite](wheeler1976cobra) model of the instrumented sub-assembly XX09. It should be noted that for SHRT-17, the power generation throughout the transient is solely due to decay heat because the protection system shuts down the reactor. For the SHRT-45R test, fission power continues into the transient for some time until the reactivity feedback mechanisms ultimately shut down the reactor.

!media figures/Normalized_Transients.png
    style=width:50%;margin-bottom:2%;margin:auto;
    id=fig:Norm_TR
    caption=Transient boundary conditions

### Improvements

The subchannel code (Pronghorn-SC) was improved by two successive corrections. The first correction involved calculating a more realistic radial and axial pin power profile, using a Serpent-2 simulation of the EBR-II core. The second correction involved calculating the heat flux from the edge subchannels to the inner duct of the thimble, using a Pronghorn-FV simulation and applying this heat flux in the Pronghorn-SC simulations. The scope of these improvements is beyond the stand-alone subchannel capabilities so they are not going to be expanded upon here.

## Steady State Results

Four simulation results are compared against the experimental measurements of thermocouples at the outlet of the heated section (TTC):

- item Results from the [DASSH](https://github.com/dassh-dev/examples/tree/master/Example-3) [!cite](atz2021ducted),subchannel code, which is used as a reference for subchannel agreement,
- item Results from Pronghorn-SC with a uniform (axially and radially) pin power profile,
- Results from Pronghorn-SC only with the corrected power profile computed via the Serpent-2 model,
- Results from Pronghorn-SC, coupled with Pronghorn-FV for determining the heat flux at the inner thimble wall, with the corrected power profile computed via the Serpent-2 model.

The DASSH subchannel code models the internal pin region of sub-assembly XX09 and the thimble region. The standalone Pronghorn-SC simulation only models the internal pin region. Both codes don't consider the neighboring sub-assemblies. The Pronghorn-FV model, includes the XX09 sub-assembly with the thimble region, the neighbor sub-assemblies, the inter-duct flow region and all the solid ducts. Thus, for the coupled simulation that couples Pronghorn-FV to Pronghorn-SC, the heat transfer effects of the thimble and neighbor assemblies, are added to the subchannel simulation.

[fig:TTC17], [fig:TTC45] present the results for the steady-state radial temperature profiles calculated at position TTC, which is near the outlet of the fueled section, for tests SHRT-17 and SHRT-45R, respectively. The figures include calculations obtained from the DASSH subchannel code. DASSH solves for the conservation of axial momentum and energy and assumes that the cross flows are produced to close the mass balance, i.e., there is no explicit equation solved for the conservation of lateral momentum in the code, and advective radial heat transfer due to cross flows is ignored. Radial thermal mixing is modeled by being lumped into the conduction term and
approximated by enhancing the thermal diffusivity with an eddy diffusivity obtained from correlation [!cite](atz2021ducted). This approximation is generally accurate for liquid-metal-cooled reactors, and hence DASSH results are used as a baseline for the performance of the subchannel code.

In the Pronghorn-SC standalone simulation with uniform power, without coupling to Pronghorn-FV, the inlet mass flow rate must be adjusted, to indirectly account for the effects of the thimble. Expert input from E. Feldman [DASSH](https://github.com/dassh-dev/examples/tree/master/Example-3) suggests that the coolant flow rate in the thimble region is roughly 8-10\% of the sub-assembly total flow rate. Based on this, the sub-assembly total flow rate of the stand-alone Pronghorn-SC model, is increased from the nominal values to $2.6923kg/s$ and $2.667kg/s$, for tests SHRT-17 and SHRT-45R, respectively. This means that the thimble flow rate is artificially rerouted through the interior of sub-assembly XX09. However, the thimble flow is significantly colder than the flow through the XX09 assembly. Thus, the approximation of adding the thimble flow to the sub-assembly, which assumes that the thimble and sub-assembly flow are in thermal equilibrium, might not be accurate. This rerouting was not necessary for the Pronghorn-SC simulations with corrected power, which uses the nominal inlet flow rates.

For SHRT-17, in the constant pin power case, both Pronghorn-SC and DASSH exhibit similar behavior. Since DASSH does not resolve the crossflows (contrary to Pronghorn-SC), similar results indicate that crossflows might not be instrumental, in determining the temperature profile for this case. Additionally, DASSH predicts a slightly less skewed distribution than Pronghorn-SC, which is closer to the experimental results. This means that the crossflows may be underestimated by the lateral momentum balance equation solved by Pronghorn-SC, or that the thimble model incorporated in DASSH is more accurate than the simplified, mass-flow adaptation applied to Pronghorn-SC. Nonetheless, both the Pronghorn-SC and DASSH calculations, are close enough to suggest that those differences in modeling approach, do not produce large discrepancies in the results.

!media figures/res_17_TTC.png
    style=width:50%;margin-bottom:2%;margin:auto;
    id=fig:TTC17
    caption=Test SHRT-17

!media figures/res_45_TTC.png
    style=width:50%;margin-bottom:2%;margin:auto;
    id=fig:TTC45
    caption=Test SHRT-45R

## Transient Results

Simulations with uniform and non-uniform power are performed with the Pronghorn-SC model. Results are compared against experimental measurements and the NETFLOW++/COBRA simulations are used as a reference result. The temperature transient calculations, together with the experimental measurements at thermocouple location TTC-31, are presented in [fig:transient1],[fig:transient2]. In SHRT-17, the power of the XX09 sub-assembly decreases rapidly at time zero due to the reactor scram, which results in a sudden coolant temperature decrease. On the other hand, in SHRT-45R there is no scram of the reactor, so power decreases at a much lower pace. This is why no sudden temperature drop is observed at time zero.

Then for both tests, the flow rate gradually decreases due to the pump trip and coasts-down, causing the coolant temperature to increase to a peak of around $890\mathrm{K}$ for SHRT-17 and $970\mathrm{K}$ for SHRT-45R. Following that, the coolant temperature decreases and levels off to $700\mathrm{K}$ for SHRT-17 and $730\mathrm{K}$ for SHRT-45R with the establishment of natural circulation due to decay heat and buoyancy effects.

For SHRT-17, the stand-alone Pronghorn-SC calculated transient, overestimates the measured result of the peak temperature, by approximately $20\mathrm{K}$, for the uniform power model and underestimates the peak temperature by approximately $5\mathrm{K}$ for the non-uniform power model. For the SHRT-45R transient, the uniform power model over-predicts the peak temperature by approximately $20\mathrm{K}$, while the non-uniform power model under-predicts the temperature, by approximately $2\mathrm{K}$. The thermal inertia of the solid structures play an important role in determining the temperature field transient, towards the establishment of natural convection. As these structures are not modeled in the Pronghorn-SC simulations, a more rapid decrease in the temperature field is observed in all cases. Nonetheless, the thermal inertia of these structures does not play a fundamental role once natural convection is established, as the system is in thermal equilibrium. Hence, better agreement is obtained for this part of the transient. In general, good results are obtained for both transients, with the non-uniform power simulations yielding better agreement with the experimental measurements.

!media figures/res_17_transient.png
    style=width:50%;margin-bottom:2%;margin:auto;
    id=fig:transient1
    caption=Test SHRT-17

!media figures/res_45_transient.png
    style=width:50%;margin-bottom:2%;margin:auto;
    id=fig:transient2
    caption=Test SHRT-45R

As previously mentioned, the SHRT-45R test is an unprotected loss of flow test, meaning that there is no reactor SCRAM. Therefore, we observe higher temperatures compared to the SHRT-17 test in which SCRAM is performed. The power transient used in the Pronghorn-SC calculation is adapted from [!cite](mochizuki2014benchmark), [!cite](mochizuki2018benchmark). Additionally, during the SHRT-45R transient, the EM pump is kept on with low power to maintain a slightly higher flow rate than natural circulation conditions. Initially, the coolant flow rate decreases, causing the temperature in the core to rise. The temperature increase leads to negative feedback due to the thermal expansion of the fuel sub-assembly, decrease of coolant density, and the Doppler effect in the nuclear cross sections. At approximately 60 seconds into the transient, the negative feedback effect becomes strong enough to significantly reduce the core power, and the temperature begins to decrease. Around 650 seconds into the SHRT-45R test, there is a sudden dip in the temperature because the auxiliary EM pump power is increased, and the coolant mass flow rate increases rapidly.

For the SHRT-17 test, both the primary coolant and intermediate-loop pumps trip, simulating a loss-of-flow accident. Additionally, the primary system auxiliary pump is deactivated, and the SCRAM signal is sent to the reactor to reduce power. The temperature increases rapidly due to the fast reduction in flow rate. As the reactor power reduces and natural convection is established, the temperature decreases toward a new steady-state value. A peak in the temperature is reached approximately 40 seconds after the start of the transient.

For both the SHRT-17 and SHRT-45R transients, the uniform power model overestimates the peak temperature. This overestimation is attributed to the fact that the effect of the thimble flow is not considered in the stand-alone subchannel model. Both simulations use the nominal boundary mass-flow rates. All of the sub-assembly-rated power is assumed to remain within the fluid in the inner sub-assembly flow area. However, applying the pin power correction alleviates this issue, resulting in better agreement with the experimental measurements.

## Input files

To run the steady state problem use the following input file:

```bash
# Following Benchmark Specifications and Data Requirements for EBR-II Shutdown Heat Removal Tests SHRT-17 and SHRT-45R
# Available at: https://publications.anl.gov/anlpubs/2012/06/73647.pdf
###################################################
#Steady state subchannel calcultion
# Thermal-hydraulics parameters
###################################################
T_in = 624.70556 #Kelvin
Total_Surface_Area = 0.000854322 #m3
mass_flux_in = ${fparse 2.45 / Total_Surface_Area} # ${fparse 2.6923 / Total_Surface_Area} #
#P_out = 43850.66 # Pa plus 4.778 meters of Na to the free surface in pool or Plus 0.57 meters of Na to core outlet.
P_out = 2.0e5
Power_initial = 486200 #W (Page 26,35 of ANL document)
# T_in = 616.4 #Kelvin
# Total_Surface_Area = 0.000854322 #m3
# mass_flux_in = ${fparse 2.667 / Total_Surface_Area} #${fparse 2.427 / Total_Surface_Area}
# #P_out = 43850.66 # Pa plus 4.778 meters of Na to the free surface in pool or Plus 0.57 meters of Na to core outlet.
# P_out = 2.0e5
# Power_initial = 379800 #W (Page 26,35 of ANL document)
###################################################
# Geometric parameters
###################################################
scale_factor = 0.01
fuel_pin_pitch = ${fparse 0.5664*scale_factor}
fuel_pin_diameter = ${fparse 0.4419*scale_factor}
wire_z_spacing = ${fparse 15.24*scale_factor}
wire_diameter = ${fparse 0.1244*scale_factor}
inner_duct_in = ${fparse 4.64*scale_factor}
n_rings = 5
heated_length = ${fparse 34.3*scale_factor}
unheated_length_exit = ${fparse 26.9*scale_factor}
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = 50
    flat_to_flat = ${inner_duct_in}
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    rod_diameter = ${fuel_pin_diameter}
    pitch = ${fuel_pin_pitch}
    dwire = ${wire_diameter}
    hwire = ${wire_z_spacing}
    spacer_z = '0.0'
    spacer_k = '0.0'
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = ${n_rings}
    n_cells = 50
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    pitch = ${fuel_pin_pitch}
  []

  [duct]
    type = TriDuctMeshGenerator
    input = fuel_pins
    nrings = ${n_rings}
    n_cells = 50
    flat_to_flat = ${inner_duct_in}
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    pitch = ${fuel_pin_pitch}
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)*exp(-alpha*z)/(1.0/alpha*(1.0 - exp(-alpha*L)))*L'
    vars = 'L alpha'
    vals = '${heated_length} 1.8012'
  []
[]

[AuxVariables]
  [mdot]
    block = subchannel
  []
  [SumWij]
    block = subchannel
  []
  [P]
    block = subchannel
  []
  [DP]
    block = subchannel
  []
  [h]
    block = subchannel
  []
  [T]
    block = subchannel
  []
  [rho]
    block = subchannel
  []
  [S]
    block = subchannel
  []
  [Sij]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [q_prime]
    block = fuel_pins
  []
  [Tpin]
    block = fuel_pins
  []
  [q_prime_duct]
    block = duct
  []
  [Tduct]
    block = duct
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  beta = 0.006
  P_out = ${P_out}
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-5
  implicit = true
  segregated = false
  interpolation_scheme = 'upwind'
[]

[ICs]
  [S_IC]
    type = TriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = TriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = TriPowerIC
    variable = q_prime
    power = ${Power_initial}
    filename = "pin_power_profile61.txt"
    # axial_heat_rate = axial_heat_rate
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = ${P_out}
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
    fp = sodium
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = sodium
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
    block = subchannel
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
  csv = true
[]

[Postprocessors]
  [TTC-27]
    type = SubChannelPointValue
    variable = T
    index = 91
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-28]
    type = SubChannelPointValue
    variable = T
    index = 50
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-29]
    type = SubChannelPointValue
    variable = T
    index = 21
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-30]
    type = SubChannelPointValue
    variable = T
    index = 4
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-31]
    type = SubChannelPointValue
    variable = T
    index = 2
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-32]
    type = SubChannelPointValue
    variable = T
    index = 16
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-33]
    type = SubChannelPointValue
    variable = T
    index = 42
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-34]
    type = SubChannelPointValue
    variable = T
    index = 80
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  [TTC-35]
    type = SubChannelPointValue
    variable = T
    index = 107
    execute_on = 'TIMESTEP_END'
    height = 0.322
  []
  # [MTC-20]
  # type = SubChannelPointValue
  # variable = T
  # index = 33
  # execute_on = 'TIMESTEP_END'
  # height = 0.172
  # []
  # [MTC-22]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 3
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.172
  # []
  # [MTC-24]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 28
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.172
  # []
  # [MTC-25]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 60
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.172
  # []
  # [MTC-26]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 106
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.172
  # []
  # [14TC-37]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 52
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.480
  # []
  # [14TC-39]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 6
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.480
  # []
  # [14TC-41]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 40
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.480
  # []
  # [14TC-43]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 105
  #   execute_on = 'TIMESTEP_END'
  #   height = 0.480
  # []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = '3d_SC_ss.i'
    execute_on = 'FINAL'
  []
[]

[Transfers]
  [subchannel_transfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []
  [pin_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin q_prime'
  []
[]

```

To run the transient problem use the following input file:

```bash
# Following Benchmark Specifications and Data Requirements for EBR-II Shutdown Heat Removal Tests SHRT-17 and SHRT-45R
# Available at: https://publications.anl.gov/anlpubs/2012/06/73647.pdf
# Transient Subchannel calculation
###################################################
# Thermal-hydraulics parameters
###################################################
T_in = 616.4 #Kelvin
Total_Surface_Area = 0.000854322 #m3
mass_flux_in = ${fparse 2.427 / Total_Surface_Area}
P_out = 2.0e5
Power_initial = 379800 #W (Page 26,35 of ANL document)
###################################################
# Geometric parameters
###################################################
scale_factor = 0.01
fuel_pin_pitch = ${fparse 0.5664*scale_factor}
fuel_pin_diameter = ${fparse 0.4419*scale_factor}
wire_z_spacing = ${fparse 15.24*scale_factor}
wire_diameter = ${fparse 0.1244*scale_factor}
inner_duct_in = ${fparse 4.64*scale_factor}
n_rings = 5
heated_length = ${fparse 34.3*scale_factor}
unheated_length_exit = ${fparse 26.9*scale_factor}
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = 50
    flat_to_flat = ${inner_duct_in}
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    rod_diameter = ${fuel_pin_diameter}
    pitch = ${fuel_pin_pitch}
    dwire = ${wire_diameter}
    hwire = ${wire_z_spacing}
    spacer_z = '0.0'
    spacer_k = '0.0'
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = ${n_rings}
    n_cells = 50
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    pitch = ${fuel_pin_pitch}
  []
[]

[AuxVariables]
  [mdot]
    block = subchannel
  []
  [SumWij]
    block = subchannel
  []
  [P]
    block = subchannel
  []
  [DP]
    block = subchannel
  []
  [h]
    block = subchannel
  []
  [T]
    block = subchannel
  []
  [rho]
    block = subchannel
  []
  [S]
    block = subchannel
  []
  [Sij]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [q_prime_init]
    block = fuel_pins
  []
  [power_history_field]
    block = fuel_pins
  []
  [q_prime]
    block = fuel_pins
  []
  [Tpin]
    block = fuel_pins
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  beta = 0.006
  P_out = ${P_out}
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-5
  T_tol = 1.0e-5
  implicit = true
  segregated = false
  interpolation_scheme = 'upwind'
[]

[ICs]
  [S_IC]
    type = TriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = TriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = TriPowerIC
    variable = q_prime_init
    power = ${Power_initial}
    filename = "pin_power_profile61_uniform.txt"
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = ${P_out}
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
    fp = sodium
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = P
    T = T
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = P
    T = T
    fp = sodium
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[Functions]
  [power_func]
    type = PiecewiseLinear
    data_file = 'power_history_SHRT45.csv'
    format = "columns"
    scale_factor = 1.0
  []
  [mass_flux_in]
    type = PiecewiseLinear
    data_file = 'massflow_SHRT45.csv'
    format = "columns"
    scale_factor = ${fparse mass_flux_in / 2.427}
  []

  [time_step_limiting]
    type = PiecewiseLinear
    xy_data = '0.1 0.1
               10.0 10.0'
  []

  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)*exp(-alpha*z)/(1.0/alpha*(1.0 - exp(-alpha*L)))*L'
    vars = 'L alpha'
    vals = '${heated_length} 1.8012'
  []
[]

[Controls]
  [mass_flux_ctrl]
    type = RealFunctionControl
    parameter = 'AuxKernels/mdot_in_bc/mass_flux'
    function = 'mass_flux_in'
    execute_on = 'initial timestep_begin'
  []
[]

[AuxKernels]
  [P_out_bc]
    type = PostprocessorConstantAux
    variable = P
    boundary = outlet
    postprocessor = report_pressure_outlet
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [mdot_in_bc]
    type = MassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = 0.0
    execute_on = 'timestep_begin'
  []
  [populate_power_history]
    type = FunctionAux
    variable = power_history_field
    function = 'power_func'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [change_q_prime]
    type = ParsedAux
    variable = q_prime
    args = 'q_prime_init power_history_field'
    function = 'q_prime_init*power_history_field'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [report_pressure_outlet]
    type = Receiver
    default = ${P_out}
  []

  [TTC-31]
    type = SubChannelPointValue
    variable = T
    index = 0
    execute_on = 'initial timestep_end'
    height = 0.322
  []

  [post_func]
    type = ElementIntegralVariablePostprocessor
    block = fuel_pins
    variable = q_prime
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient

  start_time = -1.0
  end_time = 900.0
  [TimeStepper]
    type = IterationAdaptiveDT
     dt = 0.1
     iteration_window = 5
     optimal_iterations = 6
     growth_factor = 1.2
     cutback_factor = 0.8
     timestep_limiting_function = 'time_step_limiting'
   []
   dtmax = 20
  nl_rel_tol = 0.9
  l_tol = 0.9
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
[MultiApps]
  [viz]
    type = TransientMultiApp
    input_files = '3d_SC_tr.i'
    execute_on = 'INITIAL TIMESTEP_END'
    catch_up = true
  []
[]

[Transfers]
  [subchannel_transfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []
  [pin_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin q_prime'
  []
[]

```

The corrected power profile is read by the following .txt file (pin_power_profile61.txt):

```bash
0.8477457157073588
0.8438703828994463
0.8645320077724137
0.8753092523824995
0.8645320077724137
0.8438703828994463
0.8339698482446043
0.8369432200392273
0.8566725635473428
0.8786532264515594
0.8892530859209314
0.9013443399053171
0.8892530859166047
0.8786532264515594
0.856672563539393
0.8369432200392273
0.8267424533965094
0.8177938527394076
0.8267424534001606
0.8272594696733904
0.8472934986320385
0.8681632858448316
0.8898769072293857
0.9017653027695607
0.9136133010186879
0.9253977864165255
0.9136133010118554
0.9017653027628166
0.8898769072293858
0.8681632858325796
0.8472934986203717
0.8272594696733904
0.8181228874882326
0.8089377252083733
0.7997220243775588
0.8089377252136014
0.8181228874935296
1.1303279098639667
1.1711970176131872
1.212446682465516
1.25429938376924
1.2960890595043835
1.3230893402868509
1.3483812226578455
1.3722914884147328
1.3941161594994989
1.3722914883971098
1.3483812226390315
1.3230893402669708
1.2960890595043835
1.2542993837371461
1.2124466824337514
1.1711970175818887
1.1303279098639671
1.1147644628376598
1.097730853264873
1.0797248303470215
1.0605178830397783
1.0797248303611697
1.0977308532782715
1.1147644628502191

```
