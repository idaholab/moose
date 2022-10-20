# INL Applications

[!ac](INL) develops a large number of MOOSE-based applications. These applications are typically
focused on nuclear energy, but as MOOSE and [!ac](INL) simulation capabilities continue to grow the
breadth of applications continues to expand as well. The following sections highlight some of the
[!ac](INL) developed applications.

## BISON

- [Documentation](https://mooseframework.org/bison)
- [Discussion Forum](https://bison-discourse.hpcondemand.inl.gov)
- [Local Installation](ncrc/applications/conda_bison.md)
- [HPC Binary](ncrc/applications/hpc_bison.md)

!row!
!col small=12 medium=8 large=8
BISON is a finite element-based nuclear fuel performance code. It is applicable to light water reactor fuel rods, TRISO particle fuel, metallic rod and plate fuel, and other fuel forms. BISON solves thermomechanics and species diffusion equations for 1D, 2D and 3D geometries, with fuel models that describe temperature properties, fission product swelling and other material aspects. Because it is based on the MOOSE framework, Bison can solve problems efficiently using standard workstations or very large high-performance computers. Bison is available witin our NCRC Conda channel.

!col small=8 medium=4 large=4
!media xfem/image21.gif style=width:100%;align:bottom;
!row-end!


<!-- Blue Crab goes here -->
<!-- Dire Wolf goes here -->

## Griffin

- [Documentation](https://griffin-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://griffin-discourse.hpcondemand.inl.gov)
- [Local Installation](ncrc/applications/conda_griffin.md)
- [HPC Binary](ncrc/applications/hpc_griffin.md)

!row!
!col small=12 medium=8 large=8
Griffin is a finite element-based reactor multiphysics application. It is suitable for steady state and time-dependent coupled neutronics calculations leveraging the various MOOSE-based thermal-fluids applications (Pronghorn, RELAP-7, SAM, Sockeye, etc.) and the fuel performance application (BISON). Griffin solves the linearized Boltzmann transport equation in 1D, 2D, and 3D heterogeneous and homogeneous geometries. It has been used in the analysis of pebble bed reactors (PBRs, PB-FHRs), prismatic reactors (PMRs), molten-salt reactors (MSRs), fast sodium-cooled reactors (FSRs), microreactors, nuclear thermal propulsion (NTPs), and several experimental facilities.

!col small=8 medium=4 large=4
!media application_logos/griffin_description.png style=width:100%;
!row-end!

## Grizzly

- [Documentation](https://grizzly-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://grizzly-discourse.hpcondemand.inl.gov)
- [Local Installation](ncrc/applications/conda_grizzly.md)
- [HPC Binary](ncrc/applications/hpc_grizzly.md)

!row!
!col small=12 medium=8 large=8
Grizzly models the degradation due to normal operating conditions of nuclear power plant systems, structures, and components. The code also simulates the ability of degraded components to safely perform under a variety of conditions. Grizzly can be applied to a variety of components. However, its development focused initially on the embrittlement of reactor pressure vessels and concrete structures. Vessels can degrade and facture due to irradiation and high temperatures, while concrete can degrade due to expansive alkali-silica reactions. Grizzly has capability to model the performance effect of these and other mechanisms.

!col small=8 medium=4 large=4
!media application_logos/grizzly_description.png style=width:100%;
!row-end!

## MARMOT

- [Documentation](https://marmot-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://marmot-discourse.hpcondemand.inl.gov)
- [Local Installation](ncrc/applications/conda_marmot.md)
- [HPC Binary](ncrc/applications/hpc_marmot.md)

!row!
!col small=12 medium=8 large=8
MARMOT is a mesoscale fuel performance code. As such, it can predict the evolution of the microstructure and material properties of fuels and claddings due to stress, temperature, and irradiation damage. MARMOT can, therefore, supply microstructure-based materials models to other applications that work at a larger scale, such as BISON which works at an engineering scale. MARMOT solves equations involving solid mechanics and heat conduction using the finite element method.

!col small=12 medium=4 large=4
!media application_logos/marmot_description.png style=width:100%;
!row-end!

## Pronghorn

- [Documentation](https://pronghorn-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://pronghorn-discourse.hpcondemand.inl.gov)
- [Local Installation](ncrc/applications/conda_pronghorn.md)
- [HPC Binary](ncrc/applications/hpc_pronghorn.md)

!row!
!col small=12 medium=8 large=8
Pronghorn is a multi-dimensional, coarse-mesh, thermal-hydraulics code for advanced reactors and is particularly well-suited to model gas-cooled pebble bed and prismatic reactors. It serves the intermediate fidelity realm situated between detailed computational fluid dynamics analysis and lumped system models.

!col small=12 medium=4 large=4
!row-end!

## RELAP-7

- [Documentation](https://relap7-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://relap7-discourse.hpcondemand.inl.gov)
- [Local Installation](ncrc/applications/conda_relap7.md)
- [HPC Binary](ncrc/applications/hpc_relap7.md)

!row!
!col small=12 medium=8 large=8
RELAP-7 is a two-phase thermal systems code based on MOOSE's thermal hydraulics module. RELAP-7 provides two-phase components suitable for LWRs provides closures for two-phase water from TRACE.

!col small=12 medium=4 large=4
!media application_logos/relap-7_description.png style=width:100%;
!row-end!

<!-- Sabertooth goes here -->

# Sockeye

- [Documentation](https://sockeye-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://sockeye-discourse.hpcondemand.inl.gov)
- [Local Installation](ncrc/applications/conda_sockeye.md)
- [HPC Binary](ncrc/applications/hpc_sockeye.md)

!row!
!col small=12 medium=8 large=8
Sockeye is a heat pipe analysis application geared towards heat pipes of interest in heat-pipe-cooled microreactors. Sockeye provides various transient heat pipe modeling capabilities, including a 1D, two-phase flow model and a 2D effective thermal conductivity model based on heat conduction, which leverages analytic expressions of operational limits.

!col small=12 medium=4 large=4
!media application_logos/sockeye_description.png style=width:100%;
!row-end!
