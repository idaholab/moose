
# EBR-II, SHRT-17 Validation

## Test Description

&nbsp;

 On April 3, 1986, two tests were carried out to demonstrate the effectiveness of the negative feedback and passive cooling in the EBR-II reactor. Both tests began from full reactor power and were initiated when both the primary coolant pumps and the intermediate loop pump were simultaneously tripped, to simulate a loss-of-flow accident. SHRT-17 was a protected loss-of-flow (LOF) test and SHRT-45R was an unprotected loss-of-flow (ULOF) test. At the beginning of test SHRT-17, the primary pumps were tripped at the same time than a full control rod insertion. SHRT-45R, was similar to SHRT-17 except that during this test the plant protection system (PPS) was disabled, to prevent it from initiating a control rod scram. In the second test, SHRT-45R, the reactor power decreased due to reactivity feedback effects alone.

### Plant Overview

Argonne National Laboratory (ANL)'s Experimental Breeder Reactor II (EBR-II) was a liquid-metal cooled reactor with a sodium-bonded metallic fuel core. EBR-II was rated for a thermal power of 62.5 MW with an electric output of approximately 20 MW. A schematic of the reactor and the primary sodium flow paths are shown in [fig:schematic]. All major primary system components were submerged in the primary tank, which contained approximately $340 m^3$ of liquid sodium at $371^o C$. Two primary pumps inside this pool provided sodium to the two inlet plena of the core. Sub-assemblies in the inner core received sodium from the high-pressure inlet plenum, accounting for approximately $85\%$ of the total primary flow. The blanket and reflector sub-assemblies in the outer blanket region received sodium from the low-pressure inlet plenum. Hot sodium exited the sub-assemblies into a common upper plenum, where it mixed before passing into the intermediate heat exchanger (IHX).

!media subchannel/v&v/EBR-II/EBR-II_primary_tank.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:schematic
    caption=Schematic of the EBR-II reactor [!cite](osti_801571)

The reactor-vessel grid-plenum sub-assembly accommodated 637 hexagonal sub-assemblies. The sub-assemblies were divided into three regions: core, inner blanket (IB) and outer blanket (OB). The central core comprised the 61 sub-assemblies in the first five rows. Two positions in row 3 contained safety-rod sub-assemblies and eight positions in Row 5 contained control-rod sub-assemblies. Two positions in Row 5 contained the instrumented sub-assemblies (INSAT) XX09 and XX10, and one position in Row 5 contained the in-core instrument test facility (INCOT) XY16. The remainder of the central core region contained driver fuel or experimental-irradiation sub-assemblies. EBR-II was heavily instrumented to measure mass flow rates, temperatures, and pressures, throughout the system.

SCM results are compared with data measured in the XX09 instrumented sub-assembly. More particularly, the code calculations are compared against temperature profile measurements in various axial elevations and the transient temperature evolution of the peak temperature in the central subchannel.

### Instrumented sub-assemblies

The SHRT-17 test is a protected LOF test. This test was initiated by a trip of the primary and intermediate pumps under the rated-power of 57.3MW. The reactor was scrammed at the same time as the pump trips. As flow dissipated in the primary system after the pump trips, cooling of the core transitions from forced to natural circulation, while temperature and flow rate converge to an equilibrium state.

The SHRT-45R test is an unprotected LOF test. Similarly to SHRT-17, this test was initiated by a trip of the primary and intermediate pumps under the rated-power of 60.0 MW, but without scram. In the SHRT-45R experiment, the auxiliary electromagnetic pump (EMP) was kept operational throughout the test duration. As a result, the cooling situation is not fully natural convection, but since the EMP flow rate is very low, it is also not fully forced circulation. Because of the operation of the EMP in the SHRT-45R experiment, the coolant mass flow rate converges to about two times that in the SHRT-17 experiment.

Both tests run for $900$ seconds.

This work utilizes data measured by the instrumented sub-assembly XX09. XX09 was a fueled sub-assembly specifically designed with a variety of instrumentation to provide data for benchmark validation purposes. The standard-type fueled sub-assembly contains 91 fuel pins, whereas in XX09 the outer row of fuel pins was removed and a guide thimble was inserted instead, as shown in [fig:XX09], [fig:XX09_3].

!media subchannel/v&v/EBR-II/XX09.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:XX09
    caption=XX09 Instrumented sub-assembly axial-section [!cite](summer2012benchmark)

!media subchannel/v&v/EBR-II/XX09_3.png
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

During the SHRT transients, the mass flow rate and power vary. The normalized power and mass flow rate during the transients have been taken from [!cite](mochizuki2014benchmark) , [!cite](mochizuki2018benchmark) and are presented in [fig:Norm_TR]. This information is used as input for the SCM transient calculations. The work in the cited sources utilizes a NETFLOW++ [!cite](mochizuki2010development) simulation to inform a COBRA-IV-I [!cite](wheeler1976cobra) model of the instrumented sub-assembly XX09. It should be noted that for SHRT-17, the power generation throughout the transient is solely due to decay heat because the protection system shuts down the reactor. For the SHRT-45R test, fission power continues into the transient for some time until the reactivity feedback mechanisms ultimately shut down the reactor.

!media subchannel/v&v/EBR-II/Normalized_Transients.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:Norm_TR
    caption=Transient boundary conditions

### Improvements

The subchannel module SCM (formely known as Pronghorn-SC) results, were improved by two successive off-line corrections. The first correction involved calculating a more realistic radial and axial pin power profile, using a Serpent-2 simulation of the EBR-II core. The second correction involved calculating the heat flux from the edge subchannels to the inner duct of the thimble, using a Pronghorn-FV simulation and applying this heat flux in the SCM simulations. This presentation doesn't include the Pronghorn correction and focuses only on open source codes results (SCM standalone calculations). Further information can be found in [!cite](tano2024validation).

## Steady State Results

Three simulation results are compared against the experimental measurements of thermocouples at the outlet of the heated section (TTC):

- item Results from the [DASSH](https://github.com/dassh-dev/examples/tree/master/Example-3) [!cite](atz2021ducted),subchannel code, which is used as a reference for subchannel agreement,
- item Results from SCM with a uniform (axially and radially) pin power profile,
- Results from SCM only with the corrected power profile computed via the Serpent-2 model.

The DASSH subchannel code models the internal pin region of sub-assembly XX09 and the thimble region. The standalone SCM simulation only models the internal pin region. Both codes don't consider the neighboring sub-assemblies.

[fig:TTC17], [fig:TTC45] present the results for the steady-state radial temperature profiles calculated at position TTC, which is near the outlet of the fueled section, for tests SHRT-17 and SHRT-45R, respectively. The figures include calculations obtained from the DASSH subchannel code. DASSH solves for the conservation of axial momentum and energy and assumes that the cross flows are produced to close the mass balance, i.e., there is no explicit equation solved for the conservation of lateral momentum in the code, and advective radial heat transfer due to cross flows is ignored. Radial thermal mixing is modeled by being lumped into the conduction term and approximated by enhancing the thermal diffusivity with an eddy diffusivity obtained from correlation [!cite](atz2021ducted). This approximation is generally accurate for liquid-metal-cooled reactors, and hence DASSH results are used as a baseline for the performance of the subchannel code.

For SHRT-17, in the uniform pin power case, both SCM and DASSH exhibit similar behavior. Since DASSH does not resolve the crossflows (contrary to SCM), similar results indicate that crossflows might not be instrumental, in determining the temperature profile for this problem. Additionally, DASSH predicts a slightly less skewed distribution than SCM, which is closer to the experimental results. This means that the crossflows may be underestimated by the lateral momentum balance equation solved by SCM, or that the thimble model incorporated in DASSH improves accuracy. Nonetheless, both the SCM and DASSH calculations, are close enough to suggest that those differences in modeling approach, do not produce large discrepancies in the results.

!media subchannel/v&v/EBR-II/XX09_TTC.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:TTC17
    caption=Test SHRT-17

!media subchannel/v&v/EBR-II/XX09_TTC45.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:TTC45
    caption=Test SHRT-45R

## Transient Results

Simulations with uniform power profile are performed with the SCM model. Results are compared against experimental measurements and the NETFLOW++/COBRA simulations are used as a reference result. The temperature transient calculations, together with the experimental measurements at thermocouple location TTC-31, are presented in [fig:transient1],[fig:transient2]. In SHRT-17, the power of the XX09 sub-assembly decreases rapidly at time zero due to the reactor scram, which results in a sudden coolant temperature decrease. On the other hand, in SHRT-45R there is no scram of the reactor, so power decreases at a much lower pace. This is why no sudden temperature drop is observed at time zero.

Then for both tests, the flow rate gradually decreases due to the pump trip and coasts-down, causing the coolant temperature to increase to a peak of around $892\mathrm{K}$ for SHRT-17 and $976\mathrm{K}$ for SHRT-45R. Following that, the coolant temperature decreases and levels off to $700\mathrm{K}$ for SHRT-17 and $730\mathrm{K}$ for SHRT-45R with the establishment of natural circulation due to decay heat and buoyancy effects.

For both SHRT-17 and SHRT-45R, the stand-alone SCM calculated transient, underestimates the measured result of the peak temperature, by approximately $5\mathrm{K}$. The thermal inertia of the solid structures play an important role in determining the temperature field transient, towards the establishment of natural convection. As these structures are not modeled in the SCM simulations, a more rapid decrease in the temperature field is observed in the computed cases. Nonetheless, the thermal inertia of these structures does not play a fundamental role once natural convection is established, as the system is in thermal equilibrium. Hence, better agreement is obtained for this part of the transient. In general, good results are obtained for both transients, with the SCM being in better agreement with the experiment compared to the NETFLOW++/COBRA simulations.

!media subchannel/v&v/EBR-II/Transient_Temperature.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:transient1
    caption=Test SHRT-17

!media subchannel/v&v/EBR-II/Transient_Temperature45.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=fig:transient2
    caption=Test SHRT-45R


## Input files

To run the steady state problems use the following input files:

!listing /validation/EBR-II/XX09_SCM_SS17.i language=cpp
!listing /validation/EBR-II/XX09_SCM_SS45R.i language=cpp

To run the transient problems use the following input files:

!listing /validation/EBR-II/XX09_SCM_TR17.i language=cpp
!listing /validation/EBR-II/XX09_SCM_TR45R.i language=cpp

The corrected power profile is read by the following .txt file (pin_power_profile61.txt):

!listing /validation/EBR-II/pin_power_profile61.txt language=cpp

The uniform power profile is read by the following .txt file (pin_power_profile61_uniform.txt):

!listing /validation/EBR-II/pin_power_profile61_uniform.txt language=cpp

The Functions block defines the shape of the axial power profile:

!listing /validation/EBR-II/XX09_SCM_SS17.i block=Functions language=cpp

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

!listing /validation/EBR-II/XX09_SCM_TR17.i block=Functions language=cpp

- The functions defined above are given normalized and they multiply the initial steady state conditions:

!listing /validation/EBR-II/XX09_SCM_TR17.i block=Controls language=cpp

!listing /validation/EBR-II/XX09_SCM_TR17.i block=AuxKernels language=cpp



