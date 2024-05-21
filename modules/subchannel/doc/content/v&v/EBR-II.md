
# EBR-II, SHRT-17 Validation

## Test Description

&nbsp;

 On April 3, 1986, two tests were carried out to demonstrate the effectiveness of passive feedback in the EBR-II reactor. Both tests began from full reactor power and were initiated when both the primary coolant pumps and the intermediate loop pump were simultaneously tripped, to simulate a loss-of-flow accident. SHRT-17 was a protected loss-of-flow (LOF) test and SHRT-45R was an unprotected loss-of-flow (ULOF) test. At the beginning of test SHRT-17, the primary pumps were tripped at the same time as a full control rod insertion. SHRT-45R, was similar to SHRT-17 except that during this test the plant protection system (PPS) was disabled, to prevent it from initiating a control rod scram. In the second test, reactor power decreased due to reactivity feedback effects.

 The EBR-II reactor vessel grid plenum sub-assembly accommodated 637 hexagonal sub-assemblies, which were installed in one of three regions: a central core, inner blanket, or outer blanket. The central core comprised the 61 sub-assemblies in the first five rows. SCM results are compared with data measured in the XX09 instrumented sub-assembly. More particularly, the code calculations are compared against temperature profile measurements in various axial elevations and the transient temperature evolution of the peak temperature in the central subchannel.

### Plant Overview

Argonne National Laboratory’s (ANL) Experimental Breeder Reactor II (EBR-II) was a liquid metal reactor with a sodium-bonded metallic fuel core. EBR-II was rated for a thermal power of 62.5 MW with an electric output of approximately 20 MW. A schematic of the reactor and the primary sodium flow paths are shown in [fig:schematic]. All major primary system components were submerged in the primary tank, which contained approximately $340 m^3$ of liquid sodium at $371^o C$. Two primary pumps inside this pool provided sodium to the two inlet plena of the core. Sub-assemblies in the inner core received sodium from the high-pressure inlet plenum, accounting for approximately $85\%$ of the total primary flow. The blanket and reflector sub-assemblies in the outer blanket region received sodium from the low-pressure inlet plenum. Hot sodium exited the sub-assemblies into a common upper plenum, where it mixed before passing into the intermediate heat exchanger (IHX).

!media figures/EBR-II_primary_tank.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:schematic
    caption=Schematic of the EBR-II reactor [!cite](osti_801571)

The reactor-vessel grid-plenum sub-assembly accommodated 637 hexagonal sub-assemblies. The sub-assemblies were divided into three regions: core, inner blanket (IB) and outer blanket (OB). The central core comprised the 61 sub-assemblies in the first five rows. Two positions in row 3 contained safety-rod sub-assemblies and eight positions in Row 5 contained control-rod sub-assemblies. Two positions in Row 5 contained the instrumented sub-assemblies (INSAT) XX09 and XX10, and one position in Row 5 contained the in-core instrument test facility (INCOT) XY16. The remainder of the central core region contained driver fuel or experimental-irradiation sub-assemblies. EBR-II was heavily instrumented to measure mass flow rates, temperatures, and pressures, throughout the system.

### Instrumented sub-assemblies

The SHRT-17 test is a protected LOF test. This test was initiated by a trip of the primary and intermediate pumps under the rated-power of 57.3MW. The reactor was scrammed at the same time as the pump trips. As flow dissipated in the primary system after the pump trips, cooling of the core transitions from forced to natural circulation, while temperature and flow rate converge to an equilibrium state.

The SHRT-45R test is an unprotected LOF test. Similarly to SHRT-17, this test was initiated by a trip of the primary and intermediate pumps under the rated-power of 60.0 MW, but without scram. In the SHRT-45R experiment, the auxiliary electromagnetic pump (EMP) was kept operational throughout the test duration. As a result, the cooling situation is not fully natural convection, but since the EMP flow rate is very low, it is also not fully forced circulation. Because of the operation of the EMP in the SHRT-45R experiment, the coolant mass flow rate converges to about two times that in the SHRT-17 experiment.

Both tests run for $900$ seconds.

This work utilizes data measured by the instrumented sub-assembly XX09. XX09 was a fueled sub-assembly specifically designed with a variety of instrumentation to provide data for benchmark validation purposes. The standard-type fueled sub-assembly contains 91 fuel pins, whereas in XX09 the outer row of fuel pins was removed and a guide thimble was inserted instead, as shown in [fig:XX09], [fig:XX09_3].

!media figures/XX09.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:XX09
    caption=XX09 Instrumented sub-assembly axial-section [!cite](summer2012benchmark)

!media figures/XX09_3.png
    style=width:60%;margin-bottom:2%;margin:auto;
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

During the SHRT transients, the mass flow rate and power vary. The normalized power and mass flow rate during the transients have been adapted from [!cite](mochizuki2014benchmark) , [!cite](mochizuki2018benchmark) and are presented in [fig:Norm_TR]. This information is used as input for the SCM transient calculations. The work in the cited sources utilizes a NETFLOW++ [!cite](mochizuki2010development) simulation to inform a COBRA-IV-I [!cite](wheeler1976cobra) model of the instrumented sub-assembly XX09. It should be noted that for SHRT-17, the power generation throughout the transient is solely due to decay heat because the protection system shuts down the reactor. For the SHRT-45R test, fission power continues into the transient for some time until the reactivity feedback mechanisms ultimately shut down the reactor.

!media figures/Normalized_Transients.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:Norm_TR
    caption=Transient boundary conditions

### Improvements

The subchannel module SCM (formely known as Pronghorn-SC) results, were improved by two successive corrections. The first correction involved calculating a more realistic radial and axial pin power profile, using a Serpent-2 simulation of the EBR-II core. The second correction involved calculating the heat flux from the edge subchannels to the inner duct of the thimble, using a Pronghorn-FV simulation and applying this heat flux in the SCM simulations.

## Steady State Results

Four simulation results are compared against the experimental measurements of thermocouples at the outlet of the heated section (TTC):

- item Results from the [DASSH](https://github.com/dassh-dev/examples/tree/master/Example-3) [!cite](atz2021ducted),subchannel code, which is used as a reference for subchannel agreement,
- item Results from SCM with a uniform (axially and radially) pin power profile,
- Results from SCM only with the corrected power profile computed via the Serpent-2 model,
- Results from SCM, coupled with Pronghorn-FV for determining the heat flux at the inner thimble wall, with the corrected power profile computed via the Serpent-2 model.

The DASSH subchannel code models the internal pin region of sub-assembly XX09 and the thimble region. The standalone SCM simulation only models the internal pin region. Both codes don't consider the neighboring sub-assemblies. The Pronghorn-FV model, includes the XX09 sub-assembly with the thimble region, the neighbor sub-assemblies, the inter-duct flow region and all the solid ducts. Thus, for the coupled simulation that couples Pronghorn-FV to SCM, the heat transfer effects of the thimble and neighbor assemblies, are added to the SCM simulation.

[fig:TTC17], [fig:TTC45] present the results for the steady-state radial temperature profiles calculated at position TTC, which is near the outlet of the fueled section, for tests SHRT-17 and SHRT-45R, respectively. The figures include calculations obtained from the DASSH subchannel code. DASSH solves for the conservation of axial momentum and energy and assumes that the cross flows are produced to close the mass balance, i.e., there is no explicit equation solved for the conservation of lateral momentum in the code, and advective radial heat transfer due to cross flows is ignored. Radial thermal mixing is modeled by being lumped into the conduction term and
approximated by enhancing the thermal diffusivity with an eddy diffusivity obtained from correlation [!cite](atz2021ducted). This approximation is generally accurate for liquid-metal-cooled reactors, and hence DASSH results are used as a baseline for the performance of the subchannel code.

In the SCM standalone simulation with uniform power, without coupling to Pronghorn-FV, the inlet mass flow rate must be adjusted, to indirectly account for the effects of the thimble. Expert input from E. Feldman [DASSH](https://github.com/dassh-dev/examples/tree/master/Example-3) suggests that the coolant flow rate in the thimble region is roughly 8-10\% of the sub-assembly total flow rate. Based on this, the sub-assembly total flow rate of the stand-alone SCM model, is increased from the nominal values to $2.6923kg/s$ and $2.667kg/s$, for tests SHRT-17 and SHRT-45R, respectively. This means that the thimble flow rate is artificially rerouted through the interior of sub-assembly XX09. However, the thimble flow is significantly colder than the flow through the XX09 assembly. Thus, the approximation of adding the thimble flow to the sub-assembly, which assumes that the thimble and sub-assembly flow are in thermal equilibrium, might not be accurate. This rerouting was not necessary for the SCM simulations with corrected power, which uses the nominal inlet flow rates.

For SHRT-17, in the constant pin power case, both SCM and DASSH exhibit similar behavior. Since DASSH does not resolve the crossflows (contrary to SCM), similar results indicate that crossflows might not be instrumental, in determining the temperature profile for this case. Additionally, DASSH predicts a slightly less skewed distribution than SCM, which is closer to the experimental results. This means that the crossflows may be underestimated by the lateral momentum balance equation solved by SCM, or that the thimble model incorporated in DASSH is more accurate than the simplified, mass-flow adaptation applied to SCM. Nonetheless, both the SCM and DASSH calculations, are close enough to suggest that those differences in modeling approach, do not produce large discrepancies in the results.

!media figures/res_17_TTC.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:TTC17
    caption=Test SHRT-17

!media figures/res_45_TTC.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:TTC45
    caption=Test SHRT-45R

## Transient Results

Simulations with uniform and non-uniform power are performed with the SCM model. Results are compared against experimental measurements and the NETFLOW++/COBRA simulations are used as a reference result. The temperature transient calculations, together with the experimental measurements at thermocouple location TTC-31, are presented in [fig:transient1],[fig:transient2]. In SHRT-17, the power of the XX09 sub-assembly decreases rapidly at time zero due to the reactor scram, which results in a sudden coolant temperature decrease. On the other hand, in SHRT-45R there is no scram of the reactor, so power decreases at a much lower pace. This is why no sudden temperature drop is observed at time zero.

Then for both tests, the flow rate gradually decreases due to the pump trip and coasts-down, causing the coolant temperature to increase to a peak of around $890\mathrm{K}$ for SHRT-17 and $970\mathrm{K}$ for SHRT-45R. Following that, the coolant temperature decreases and levels off to $700\mathrm{K}$ for SHRT-17 and $730\mathrm{K}$ for SHRT-45R with the establishment of natural circulation due to decay heat and buoyancy effects.

For SHRT-17, the stand-alone SCM calculated transient, overestimates the measured result of the peak temperature, by approximately $20\mathrm{K}$, for the uniform power model and underestimates the peak temperature by approximately $5\mathrm{K}$ for the non-uniform power model. For the SHRT-45R transient, the uniform power model over-predicts the peak temperature by approximately $20\mathrm{K}$, while the non-uniform power model under-predicts the temperature, by approximately $2\mathrm{K}$. The thermal inertia of the solid structures play an important role in determining the temperature field transient, towards the establishment of natural convection. As these structures are not modeled in the SCM simulations, a more rapid decrease in the temperature field is observed in all cases. Nonetheless, the thermal inertia of these structures does not play a fundamental role once natural convection is established, as the system is in thermal equilibrium. Hence, better agreement is obtained for this part of the transient. In general, good results are obtained for both transients, with the non-uniform power simulations yielding better agreement with the experimental measurements.

!media figures/res_17_transient.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:transient1
    caption=Test SHRT-17

!media figures/res_45_transient.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:transient2
    caption=Test SHRT-45R

As previously mentioned, the SHRT-45R test is an unprotected loss of flow test, meaning that there is no reactor SCRAM. Therefore, we observe higher temperatures compared to the SHRT-17 test in which SCRAM is performed. The power transient used in the SCM calculation is adapted from [!cite](mochizuki2014benchmark), [!cite](mochizuki2018benchmark). Additionally, during the SHRT-45R transient, the EM pump is kept on with low power to maintain a slightly higher flow rate than natural circulation conditions. Initially, the coolant flow rate decreases, causing the temperature in the core to rise. The temperature increase leads to negative feedback due to the thermal expansion of the fuel sub-assembly, decrease of coolant density, and the Doppler effect in the nuclear cross sections. At approximately 60 seconds into the transient, the negative feedback effect becomes strong enough to significantly reduce the core power, and the temperature begins to decrease. Around 650 seconds into the SHRT-45R test, there is a sudden dip in the temperature because the auxiliary EM pump power is increased, and the coolant mass flow rate increases rapidly.

For the SHRT-17 test, both the primary coolant and intermediate-loop pumps trip, simulating a loss-of-flow accident. Additionally, the primary system auxiliary pump is deactivated, and the SCRAM signal is sent to the reactor to reduce power. The temperature increases rapidly due to the fast reduction in flow rate. As the reactor power reduces and natural convection is established, the temperature decreases toward a new steady-state value. A peak in the temperature is reached approximately 40 seconds after the start of the transient.

For both the SHRT-17 and SHRT-45R transients, the uniform power model overestimates the peak temperature. This overestimation is attributed to the fact that the effect of the thimble flow is not considered in the stand-alone SCM model. Both simulations use the nominal boundary mass-flow rates. All of the sub-assembly-rated power is assumed to remain within the fluid in the inner sub-assembly flow area. However, applying the pin power correction alleviates this issue, resulting in better agreement with the experimental measurements.

## Input files

To run the steady state problem use the following input file:

!listing /examples/EBR-II/XX09_SC_SS.i language=cpp

To run the transient problem use the following input file:

!listing /examples/EBR-II/XX09_SC_tr.i language=cpp

The corrected power profile is read by the following .txt file (pin_power_profile61.txt):

!listing /examples/EBR-II/pin_power_profile61.txt language=cpp

The Functions block defines the shape of axial power profile:

!listing /examples/EBR-II/XX09_SC_SS.i block=Functions language=cpp

### Transient BC's

For the transient case the user needs to provide transient boundary conditions:

- First define the initial power and the power history field along with the default variable q_prime, in the AuxVariables block

```language=bash
  [q_prime_init]
    block = fuel_pins
  []
  [power_history_field]
    block = fuel_pins
  []
  [q_prime]
    block = fuel_pins
  []

```

- Then, define the transient evolution of the boundary conditions in the Functions block based on .csv files:

!listing /examples/EBR-II/XX09_SC_tr.i block=Functions language=cpp

- The functions defined above are given normalized and they need to multiply the initial steady state conditions:

```language=cpp
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

```

- Finally the Executioner block defines the time step with the TimeStepper:

!listing /examples/EBR-II/XX09_SC_tr.i block=Executioner language=cpp

### Coupling to Pronghorn

A porous media Pronghorn-FV simulation is performed to calculate the inter-assembly heat transfer at the inner duct of the XX09 sub-assembly. Pronghorn-FV uses a full porous media model of the XX09 and the neighboring sub-assemblies. The computed heat flux at the inner wall of the inner duct of sub-assembly XX09 is then transferred from Pronghorn-FV to SCM, via the [MutliApp](https://mooseframework.inl.gov/syntax/MultiApps/) fuctionality in [MOOSE](https://mooseframework.inl.gov). Running the SCM simulation with the heat-flux information further improves the fidelity of the results.

The Pronghorn-FV model for SHRT-17 is depicted in [fig:coupling] along with the subchannel pin mesh (SCM model). The sub-assembly XX09 is modeled along with its six neighboring sub-assemblies. The model includes the thimble region of XX09, inter-wrapper, and duct walls. In counter-clockwise order, the six neighboring sub-assemblies include two driver sub-assemblies (D1 and D2). Next in order is the steel sub-assembly (KO11). Next, experimental sub-assembly (X402). Next, High-Flow Driver sub-assembly (HFD) and last a 1/2 driver sub-assembly (P).

!media figures/coupling.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:coupling
    caption=Test SHRT-17, Pronghorn-FV and SCM models (domain overllaping).

The porosity of each sub-assembly can be calculated using the dimensions provided [!cite](summer2012benchmark) and it is defined as the volume of the flow region over the total flow of the sub-assembly (flow region plus solid structures). For SHRT-17, XX09 has a porosity of $0.45847$, K011 $0.15663$ and the rest $0.44785$. Pronghorn-FV calculates the heat conduction in the solid region (duct walls) and the porous flow in the fluid regions (sub-assemblies, inter-assembly region, and thimble) and couples both solution fields, using conjugate heat-transfer models. However, the main purpose of the Pronghorn-FV simulation is to calculate the linear heat-flux $W/m$ on the interface between XX09 and the inner duct of the thimble. This information is passed on to the SCM calculation via the `MultiApp` functionality of `MOOSE`. This constitutes a one-way coupling between Pronghorn-FV and SCM.

- The input file that produces the Pronghorn-FV mesh is:

!listing /examples/EBR-II/EBR-II_7assemblies.i language=cpp

- The input file for the coupled simulation is:

!listing /examples/EBR-II/Pr_SC_ss.i language=cpp
