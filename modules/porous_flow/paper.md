---
title: 'PorousFlow: a multiphysics simulation code for coupled problems in porous media'
tags:
  - C++
  - porous flow
  - multiphysics
  - THMC
  - Darcy flow
authors:
  - name: Andy Wilkins
    orcid: 0000-0001-6472-9560
    affiliation: 1
  - name: Christopher P. Green
    orcid: 0000-0002-7315-6597
    affiliation: 1
  - name: Jonathan Ennis-King
    orcid: 0000-0002-4016-390X
    affiliation: 1
affiliations:
  - name: CSIRO (Commonwealth Scientific and Industrial Research Organisation)
    index: 1
date: 13 February 2020
bibliography: paper.bib
---

# Summary

``PorousFlow`` enables simulation of transport and flow in porous media.  These are important in many practical fields of research.  For instance, ``PorousFlow`` has been used in the following applications.

- Groundwater studies [@herron; @crosbie].  This often involves assessing the impacts of human interventions on groundwater resources, and the consequences on the biosphere.  It is frequently necessary to use large and complex models that include the effect of rainfall patterns, spatially and temporally varying evaporation and transpiration, seasonal river flows, realistic topography and hydrogeology, as well as human factors such as mines, water bores, dams, etc.  In certain situations, such as assessing the impact of mining on groundwater, it is vital that the model includes the deformation of the porous material (the rock) since this greatly impacts groundwater flow.  In other situations, it is useful to be able to track the transport of tracers and pollutants through the system, or to model the unsaturated (vadose) zone, where air and groundwater coexist in the subsurface.

- Geothermal modelling: the simulation of natural and man-made geothermal systems, typically in order to quantify heat energies that may be extracted [@birdsall; @guglielmetti; @lima; @niederau; @jin].  Models in this field involve the coupled transport of heat and fluid.  Many models are often large-scale, but some focus on small regions close to wellbores to investigate wellbore integrity.  In this field, it often is useful to assess the impacts of subsurface heterogeneity, realistic pumping regimes, geochemical effects, fracture flow, and the creation of new fractures through deliberate or unintentional hydrofracturing, as well as potential microseismicity.

- Multiphase modelling, such as the simulation of CO$_{2}$ sequestration [@green2018; @lima], H$_{2}$ storage, or methane flows around coal mines [@c26050; @c25065].  These simulations involve the interaction and flow of at least two fluid phases (e.g. gas and water).  Changes in temperature often impact these flows, and realistic equations of state for the fluids involved are necessary.  For instance, CO$_{2}$ sequestration typically involves pumping high-pressure, cold CO$_{2}$ into a warmer aquifer filled with brine: the CO$_{2}$ dissolves into the brine, cooling it and changing its density and flow characteristics.  In many of these models, the altered pressure and temperature can potentially cause mechanical failure of the subsurface, and hence the creation of new flow paths and microseismicity [@lima]

- Understanding mineralization [@heather1; @heather2] in the subsurface often involves fluid flow, heat flow and coupled 3D deformation, simulating over millennia of real time and using realistic, and therefore complicated, basin-scale geology.

Other situations where ``PorousFlow`` could be used include oil and gas production, nuclear waste repositories and chemical leaching.

In many of these cases it is not enough to simply solve simplistic, traditional flow and transport physics.  Coupling with other physics is necessary: solid mechanical deformations and stresses can be important; geochemistry can alter the flow characteristics of the subsurface; high-precision equations of state are required; the evolution of fluid components (tracers, pollutants, reactants) is of interest; unsaturated physics is crucial.  The flow and transport often impacts the subsurface structure, and conversely, the effects of these structure-changes impact the flow and transport.

Simulating these situations can be challenging in practice.  Many widely-used simulation codes typically focus on only a subset of physical processes, and therefore accurate modelling often requires the use of several loosely-coupled software packages.  Such an approach can be useful in many problems of interest, particularly where the characteristic time scales for each physical process are sufficiently different.  In other instances, however, the ability to tightly couple different physical processes is necessary due to comparable characteristic time scales for the different physical processes.
``PorousFlow`` allows simulation of all the relevant thermal-hydraulic-mechanical-chemical (THMC) physical phenomenon in a tightly-coupled framework.

Other open-source codes that can also perform tightly-coupled THMC simulations to some degree include OpenGeoSys [@opengeosys], PFLOTRAN [@pflotran] and work under the Open Porous Media initiative [@opm].

``PorousFlow`` is built upon the open-source, massively parallel, fully implicit multiphysics simulation framework MOOSE (Multiphysics Object-Oriented Simulation Environment) [@permann2019moose].  MOOSE is an open-source library from Idaho National Laboratory that provides a high-level interface to the libMesh finite element library [@libmesh] and PETSc nonlinear solvers [@petsc].  As such, it runs well on the smallest notebook computer through to the world's largest supercomputers.  MOOSE and ``PorousFlow`` follow [strict quality controls](https://mooseframework.org/sqa/index.html).

For convenience, the source code for ``PorousFlow``, https://github.com/idaholab/moose/tree/master/modules/porous_flow, is bundled within the MOOSE framework, https://github.com/idaholab/moose, and detailed documentation found at https://mooseframework.org/modules/porous_flow/index.html.

# Acknowledgements

The authors wish to acknowledge other developers who have contributed to the ``PorousFlow`` module, especially Heather Sheldon and Philipp Schaedle.  Funding for the development was obtained through CSIRO strategic funds, sourced by Jonathan Ennis-King, Deepak Adhikary and Qingdong Qu, as well as funds from the Idaho National Laboratory through Robert Podgorney.  The authors thank the MOOSE framework team, past and present, for providing the MOOSE framework and auxiliary functionality (quality control, test harnesses, documentation scaffolds, build scripts, etc).

# References
