# NCRC Distributed Applications

NCRC distributes a number of NEAMS-developed applications. The following sections highlight some of the codes available through the NCRC.

## BISON

- [Documentation](https://mooseframework.org/bison)
- [Discussion Forum](https://bison-discourse.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_bison_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_bison.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_bison.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/bison)

!row!
!col small=12 medium=8 large=8
BISON is a finite element-based nuclear fuel performance code. It is applicable to light water reactor fuel rods, TRISO particle fuel, metallic rod and plate fuel, and other fuel forms. BISON solves thermomechanics and species diffusion equations for 1D, 2D and 3D geometries, with fuel models that describe temperature properties, fission product swelling and other material aspects. Because it is based on the MOOSE framework, Bison can solve problems efficiently using standard workstations or very large high-performance computers. Bison is available witin our NCRC Conda channel.

!col small=8 medium=4 large=4
!media xfem/image21.gif style=width:100%;align:bottom;
!row-end!

## Blue Crab

- [Documentation](https://bluecrab-dev.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_bluecrab_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_bluecrab.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_bluecrab.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/blue_crab)

!row!
!col small=12 medium=8 large=8
Blue Crab is a combination (coupling) of other NCRC Applications: Bison, Griffin, Pronghorn and SAM.

!col small=8 medium=4 large=4
!row-end!

## Dire Wolf

- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_direwolf_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_direwolf.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_direwolf.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/dire_wolf)

!row!
!col small=12 medium=8 large=8
Dire Wolf is a combination (coupling) of other NCRC Applications: Bison, Griffin and Sockeye.

!col small=8 medium=4 large=4
!row-end!


## Griffin

- [Documentation](https://griffin-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://griffin-discourse.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_griffin_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_griffin.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_griffin.md)
- [Level 4 - Source Access](https://github.inl.gov/ncrc/griffin)

!row!
!col small=12 medium=8 large=8
Griffin is a finite element-based reactor multiphysics application. It is suitable for steady state and time-dependent coupled neutronics calculations leveraging the various MOOSE-based thermal-fluids applications (Pronghorn, RELAP-7, SAM, Sockeye, etc.) and the fuel performance application (BISON). Griffin solves the linearized Boltzmann transport equation in 1D, 2D, and 3D heterogeneous and homogeneous geometries. It has been used in the analysis of pebble bed reactors (PBRs, PB-FHRs), prismatic reactors (PMRs), molten-salt reactors (MSRs), fast sodium-cooled reactors (FSRs), microreactors, nuclear thermal propulsion (NTPs), and several experimental facilities.

!col small=8 medium=4 large=4
!media application_logos/griffin_description.png style=width:100%;
!row-end!

## Grizzly

- [Documentation](https://grizzly-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://grizzly-discourse.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_grizzly_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_grizzly.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/grizzly)

!row!
!col small=12 medium=8 large=8
Grizzly models the degradation due to normal operating conditions of nuclear power plant systems, structures, and components. The code also simulates the ability of degraded components to safely perform under a variety of conditions. Grizzly can be applied to a variety of components. However, its development focused initially on the embrittlement of reactor pressure vessels and concrete structures. Vessels can degrade and facture due to irradiation and high temperatures, while concrete can degrade due to expansive alkali-silica reactions. Grizzly has capability to model the performance effect of these and other mechanisms.

!col small=8 medium=4 large=4
!media application_logos/grizzly_description.png style=width:100%;
!row-end!

## MARMOT

- [Documentation](https://marmot-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://marmot-discourse.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_marmot_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_marmot.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_marmot.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/marmot)

!row!
!col small=12 medium=8 large=8
MARMOT is a mesoscale fuel performance code. As such, it can predict the evolution of the microstructure and material properties of fuels and claddings due to stress, temperature, and irradiation damage. MARMOT can, therefore, supply microstructure-based materials models to other applications that work at a larger scale, such as BISON which works at an engineering scale. MARMOT solves equations involving solid mechanics and heat conduction using the finite element method.

!col small=8 medium=4 large=4
!media application_logos/marmot_description.png style=width:100%;
!row-end!

## Pronghorn

- [Documentation](https://pronghorn-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://pronghorn-discourse.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_pronghorn_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_pronghorn.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_pronghorn.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/pronghorn)

!row!
!col small=12 medium=8 large=8
Pronghorn is a multi-dimensional, coarse-mesh, thermal-hydraulics code for advanced reactors and is particularly well-suited to model gas-cooled pebble bed and prismatic reactors. It serves the intermediate fidelity realm situated between detailed computational fluid dynamics analysis and lumped system models.

!col small=8 medium=4 large=4
!row-end!

## RELAP5-3D

- [Documentation](https://relap53d.inl.gov/SitePages/Home.aspx)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_relap5_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_relap5.md)

!row!
!col small=12 medium=8 large=8
RELAP5-3D is the latest released code in the RELAP5 series. Developed at Idaho National Laboratory, the RELAP5 family aids the analysis of transients and accidents in water-cooled nuclear power plants and related systems. The software can also analyze advanced reactor designs. RELAP5-3D differs from earlier code in the series because it offers fully integrated and multi-dimensional thermal-hydraulic and kinetic modeling. It runs on both Linux and Windows operating systems, with training and users group available.

!col small=8 medium=4 large=4
!row-end!

## RELAP-7

- [Documentation](https://relap7-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://relap7-discourse.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_relap7_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_relap7.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_relap7.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/relap-7)

!row!
!col small=12 medium=8 large=8
RELAP-7 is a two-phase thermal systems code based on MOOSE's thermal hydraulics module. RELAP-7 provides two-phase components suitable for LWRs provides closures for two-phase water from TRACE.

!col small=8 medium=4 large=4
!media application_logos/relap-7_description.png style=width:100%;
!row-end!

## Sabertooth

- [Documentation](https://sabertooth-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://sabertooth-discourse.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_sabertooth_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_sabertooth.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_sabertooth.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/sabertooth)

!row!
!col small=12 medium=8 large=8
Sabertooth is a combination (coupling) of other NCRC Applications: Bison, Griffin, and Relap-7.

!col small=8 medium=4 large=4
!row-end!

## SAM

- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_sam_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_sam.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/sam)

!row!
!col small=12 medium=8 large=8
System Analysis Module (SAM) is a fast-running, whole-plant transient analysis code with improved-fidelity capability for fast turnaround design scoping and safety analyses of advanced non-light-water reactors.

!col small=8 medium=4 large=4
!row-end!

## Sockeye

- [Documentation](https://sockeye-docs.hpcondemand.inl.gov/latest/)
- [Discussion Forum](https://sockeye-discourse.hpcondemand.inl.gov)
- [Level 1 - HPC OnDemand Execution](ncrc/applications/ncrc_sockeye_ondemand.md)
- [Level 1 - HPC Binary Execution](ncrc/applications/hpc_sockeye.md)
- [Level 2 - Local Binary Installation](ncrc/applications/conda_sockeye.md)
- [Level 4 - Source Access](https://hpcgitlab.hpcondemand.inl.gov/idaholab/sockeye)

!row!
!col small=12 medium=8 large=8
Sockeye is a heat pipe analysis application geared towards heat pipes of interest in heat-pipe-cooled microreactors. Sockeye provides various transient heat pipe modeling capabilities, including a 1D, two-phase flow model and a 2D effective thermal conductivity model based on heat conduction, which leverages analytic expressions of operational limits.

!col small=8 medium=4 large=4
!media application_logos/sockeye_description.png style=width:100%;
!row-end!
