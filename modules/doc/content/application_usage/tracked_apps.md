# Applications leveraging MOOSE

MOOSE is designed for building custom applications, the following is a community driven list of applications.

The purpose of these lists is two fold:

1. Every MOOSE-based application is a library, thus it is natural to create new applications from existing code.
1. The MOOSE developers help keep many applications up to date. But we have to know your application exists to do so!

## Open-source Applications

### Multiphysics frameworks

- [Cardinal](https://github.com/neams-th-coe/cardinal) - Integration of NekRS \& OpenMC with MOOSE for fusion and fission systems high-fidelity simulation
- [SALAMANDER](https://github.com/idaholab/salamander) - Integration of Cardinal with TMAP8 for fusion blanket multiphysics simulations


### Neutronics / Reactor / Particle Physics

- [Aurora](https://aurora-multiphysics.github.io/aurora/) - Integration of OpenMC and MOOSE developed by the UKAEA
- [Frog](https://github.com/idaholab/thermal_to_fusion_converter) - Integration of GEANT-4 for high-energy particle transport
- [Moltres](https://github.com/arfc/moltres) - Molten salt reactor simulation
- [Squirrel](https://github.com/philipJFpfahl/Squirrel) - Point kinetics with delayed neutron precursor advection in molten salt reactors


### Thermal hydraulics and fluid dynamics

- [OpenPronghorn](https://mooseframework.inl.gov/open_pronghorn/) - Coarse mesh computational fluid dynamics
- [barnacle](https://github.com/idaholab/barnacle) - Shallow water equations in MOOSE
- [hippo](https://github.com/aurora-multiphysics/hippo) - Integration of OpenFOAM and MOOSE to enable thermohydraulic simulations
- [Saline](https://code.ornl.gov/neams/saline) - Molten salt fluid properties from MSTDB experimental data
- [Marlin](https://github.com/idaholab/marlin) - Spectral PDE and Lattice Boltzmann solver based on MOOSE with GPU-acceleration


### Mechanics

- [Blackbear](https://github.com/idaholab/blackbear) - Degradation processes in concrete and other structural materials
- [Deer](https://github.com/Argonne-National-Laboratory/deer) - Various mechanics modules for MOOSE
- [MASTODON](https://github.com/idaholab/mastodon) - Structural dynamics, seismic analysis, and risk assessment
- [RACCOON](https://github.com/hugary1995/raccoon) - Massively parallel FEM code for phase-field for fracture
- [Okami](https://hpcgitlab.hpc.inl.gov/idaholab/okami/) - Mixed Oxide fuel studies

### Materials

- [GRIME](https://github.com/shortlab/grime) - The Grand Radiation Informed Microstructural Evolver: Coupled, spatially dependent radiation damage and microstructural evolution
- [HOGNOSE](https://github.com/shortlab/hognose) - CASL's mesoscale corrosion/oxidation code
- [Hyrax](https://github.com/UMThorntonGroup/Hyrax) - Zirconium hydride precipitation and growth in LWR nuclear fuel cladding
- [Magpie](https://github.com/idaholab/magpie) - Mesoscale Atomistic Glue Program for Integrated Execution
- [Pika](https://github.com/idaholab/pika) - Phase-field model for micro-structure evolution of ice


### Electromagnetics, Plasmas and Additive Manufacturing

- [Apollo](https://github.com/aurora-multiphysics/apollo) - Enabling 3D electromagnetics simulation in MOOSE, using the MFEM FE library.
- [CRANE](https://crane-plasma-chemistry.readthedocs.io/en/latest/) - Chemical ReAction NEtworks for plasma chemistry and thermochemistry problems.
- [Ferret](https://mangerij.github.io/ferret/) - Kernels to implement the time-dependent Landau-Ginzburg theory of phase transitions for simulating ferroelectric materials
- [MALAMUTE](https://mooseframework.inl.gov/malamute/) - Advanced manufacturing modeling and simulation
- [Orpheus](https://github.com/aurora-multiphysics/orpheus) - Additional electromagnetics solvers in MOOSE
- [phaethon](https://github.com/aurora-multiphysics/phaethon) - Fast ion heat fluxes produced by ASCOT5 in MOOSE multiphysics simulations
- [Zapdos](https://github.com/shannon-lab/zapdos) - Low temperature plasma simulation


### Chemistry & species transport

- [DGOSPREY](https://github.com/aladshaw3/dgosprey) - Discontinuous Galerkin Off-gas SeParation and REcoverY model: joint development between INL and GIT
- [TMAP8](https://mooseframework.inl.gov/TMAP8/) - System-level mass and thermal transport calculations related to tritium migration.


### Geomechanics, Ground Flow, and Earth Sciences

- [FalCon](https://github.com/idaholab/falcon) - Fracturing and liquid conservation; geothermal reservoir simulation and analysis code for coupled and fully implicit Thermo-Hydro-Mechanical-Chemical (THMC) geosystems
- [Redback](https://github.com/pou036/redback) - Rock Mechanics with Dissipative (Thermo-Hydro-Mechanical-Chemical) Feedbacks: joint development between CSIRO and UNSW Australia


### Miscellaneous

- [moopy](https://github.com/aurora-multiphysics/moopy) - Python interface that simplifies (ish) the creation of MOOSE inputs
- [Proteus](https://github.com/aurora-multiphysics/proteus) - For developing Fusion Digital Twins.
- [Ranger](https://github.com/idaholab/ranger) - Python-based auto-response bot that uses the GitHub API and LlamaIndex package to monitor and generate relevant responses for new discussions
- [Virtual Test Bed](https://mooseframework.inl.gov/virtual_test_bed/) - An open repository of simulations of nuclear systems
- [Isopod](https://github.com/idaholab/isopod) - Multiphysics PDE constrained optimization, mostly merged into MOOSE as the optimization module


## Closed Source Applications

These applications require a license. Many of them can be obtained through the [NCRC website](https://ncrcaims.inl.gov/).

### Nuclear Engineering Advanced Modeling and Simulation (NEAMS) tools

- [BISON](https://inlsoftware.inl.gov/product/bison) - Flagship fuels performance code
- [Centipede](https://www.sciencedirect.com/science/article/pii/S0022311520301884) - A tool to inform engineering scale simulations with atomistic data, developed at LANL
- [GRIFFIN](https://inlsoftware.inl.gov/product/griffin) - Reactor physics code co-developed between Argonne and Idaho National Laboratory
- [GRIZZLY](https://inlsoftware.inl.gov/product/grizzly) - Nuclear Plant System Degradation Modeling
- [MARMOT](https://inlsoftware.inl.gov/product/marmot) - Nuclear materials phase field and mechanics application
- [PRONGHORN](https://inlsoftware.inl.gov/product/pronghorn) - Transient prismatic and pebble bed reactor analysis code
- [RELAP7](https://inlsoftware.inl.gov/product/relap7) - Next Generation nuclear reactor system safety analysis code (compressible flow)
- [SAM](https://www.anl.gov/nse/system-analysis-module) - Advanced nuclear reactor system analysis code (weakly-compressible flow), developed at ANL
- [Sockeye](https://inlsoftware.inl.gov/product/sockeye) - Multi-fidelity studies of heat pipes for nuclear applications
- SWIFT - A thermo-chemistry tool for metal hydride moderators, developed at LANL

Combined applications

- [DIREWOLF](https://www.tandfonline.com/doi/full/10.1080/00295450.2021.1906474) - Advanced reactor multiphysics simulation suite including Griffin, Bison, Sockeye
- BLUECRAB - Advanced reactor multiphysics simulation suite including Griffin, Bison, Pronghorn, SAM (and Sockeye for special builds)
- SABERTOOTH - Advanced reactor multiphysics simulation suite

### Others

- Great White - 3D discrete dislocation dynamic application using the Mechanics of Defects Library
- Mixcoatl$^{TM}$ - Conjugate heat transfer in microreactors
- MONARCH - Radiation damage modeling in semiconductors


## Legacy software

We list here older, unmaintained, projects for consideration when naming a new project.

### Open source

- [Achlys](https://github.com/aurora-multiphysics/achlys) - Macroscopic tritium transport processes through fusion materials
- [Dendragapus](https://github.com/jarons/dendragapus) - Application to explore modifications to Picard Iteration
- [Gardensnake](https://github.com/friedmud/gardensnake) - Nodal neutron diffusion code developed at MIT
- [Slug](https://github.com/adamLange/slug) - Hyperloop air bearing simulation tool
- [MaCaw](https://github.com/idaholab/macaw) - Domain-decomposed unstructured mesh Monte Carlo particle transport using OpenMC as a collision physics library
- [MAMBA](https://github.com/shortlab/mamba) - MOOSE port of the still maintained non-MOOSE MAMBA CASL code for boiling, heat transfer, and chemistry in porous media
- [Crow](https://github.com/SudiptaBiswas/Crow) - MOOSE-based mesoscale simulations for solid-state sintering.


### Closed source

- BIGHORN - Compressible fluid dynamics code
- ELK - Former application for the electromagnetics module
- FENIX - Multiphysics simulation suite with Griffin and Pronghorn, including Xenon and Samarium effects
- MAMMOTH - Reactor physics application
- OSPREY - Off-gas SeParation and REcoverY for dispersed plug flow in a packed bed.
- RATTLESNAKE - FEM Diffusion, SN, and PN code
- YAK - Common object library for INL particle transport codes
- YellowJacket - Nuclear reactor coolant chemistry
