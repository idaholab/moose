# Thermal Hydraulics Applications

There are several MOOSE-based applications modeling thermal hydraulics, which
are developed as part of the
[Nuclear Energy Advanced Modeling and Simulation (NEAMS) program](https://neams.inl.gov/).
These applications provide capabilities at various levels of fidelity vs.
computational efficiency for a multi-layered simulation strategy. The following
table summarizes these applications, in order of decreasing fidelity.

| Application | Description | Link(s) |
| :- | :- | :- |
| Pronghorn | Coarse-mesh thermal hydraulics application targeting advanced reactor analysis. | [Website](https://pronghorn-dev.hpc.inl.gov/site/index.html) |
| Subchannel | Sub-channel application for performing reactor core, single-phase thermal-hydraulic simulations, for water-cooled, bare rod, square lattice bundles or metal-cooled (sodium, lead), wire-wrapped/bare rod, triangular lattice bundles. | [Website](https://subchannel-dev.hpc.inl.gov/site/index.html) |
| THM | The MOOSE Thermal Hydraulics Module. THM provides a framework for composable thermal-hydraulic systems simulations, as well as a library of 1D, single-phase, compressible flow model components including pipes, junctions, valves, and turbomachinery, as well as 2D and 3D solid bodies. | [Website](modules/thermal_hydraulics/index.md) |
| SAM | Thermal hydraulics systems code for advanced non-light-water reactors. |  |
| Sockeye | Simulates high-temperature heat pipes with multiple 1D and 2D models. | [Website](https://sockeye-dev.hpc.inl.gov/site/) |

Most of these applications can be obtained through INL's [Nuclear Computational Resource Center (NCRC)](https://inl.gov/ncrc/); see [help/inl/applications.md] for more information.
