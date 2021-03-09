# Simulating underground coal mining

## Introduction

In an underground longwall coal mine, coal is mined in "panels", as shown in [lw_mining_graphic.fig].
These panels are typically 3--4$\,$m in height, 150--400$\,$m in width and 1000--4000$\,$m long.
Coal is extracted from one end, moving towards the other end.  As mining progresses, a void is
created behind the mining machinery where the coal used to be.  The rock material above this void---
the "overburden"---is not strong enough to support itself, so it collapses downwards.  The resulting
partial void and collapsed material is called the "goaf" (or "gob").

!media media/porous_flow/lw_mining_graphic.png
       id=lw_mining_graphic.fig
       style=width:80%;margin-left:10px;
       caption=Pictorial representation of a single panel in longwall mining. The dimensions and
               process are described in the text.  Figure sourced from
               citizensagainstlongwallmining.org who obtained it from the Energy Information
               Administration.

There are many geotechnical aspects that are of interest in this
process, but this Example concentrates on the following.

1. The vertical displacement of the ground surface due to the longwall mining.  This is called the
   "subsidence".  This obviously has effects on buildings and other man-made structures like roads,
   but it can also change surface water pathways, affect vegetation, and cause surface-water ponding,
   which all have effects on the local ecology.

2. The vertical deformation within the overburden.

3. The patterns of fracture in the overburden.  Together with the previous item, this can have
   effects on operational aspects of longwall mining, such as the load patterns on the "chocks" that
   support the roof of the goaf next to the mining machinery.  It also strongly effects the flow of
   fluids in the porous rocks that make up the overburden, as discussed next.

4. The flow of water through the rock.  If the fracturing and deformation is extensive then fluids
   can easily move through the rock.  This can result in mines flooding with water, or causing
   excessive drawdown of groundwater aquifers which can effect other users of groundwater (such as
   irrigators for farming) or can effect the rates of groundwater baseflow to surrounding river
   systems which may effect local and not-so-local ecosystems.  The deformation and fracture of the
   overburden can also lead to aquifer mixing, where an aquifer consisting of "dirty" water (with
   high salinity, for example) pollutes a nearby aquifer of clean water.  Although not studied in
   this Example, the deformation and fracture can also lead to large releases of methane gas from
   overlying (and underlying) coal seams.  This methane can then flow through the highly permeable
   fractured rock system to the atmosphere, resulting in large greenhouse gas emissions.  Or it can
   flow to the mine workings which is extremely hazardous in terms of mine fires and explosions.

This Example does not seek to build a realistic model of a specific mine.  Instead, it explains how
the PorousFlow module can be used to build such a model, by exploring a simple 3D model.

## The model

Two input files are provided with this example:
[coarse_with_fluid.i](https://github.com/idaholab/moose/blob/master/modules/porous_flow/examples/coal_mining/coarse_with_fluid.i)
and
[fine_with_fluid.i](https://github.com/idaholab/moose/blob/master/modules/porous_flow/examples/coal_mining/fine_with_fluid.i).
They essentially only differ in the input mesh.  The results below are drawn from the fine model, but
it takes a few hours to complete on a computer cluster, so the coarse model might be a better model
to explore initially.

### Geometry

[3D.fig] shows the geometrical setup as well as the fine mesh.  The model represents a single
longwall panel of total width 300$\,$m and length 1000$\,$m.  Only half of the width is modelled, and
appropriate boundary conditions used to simulate the full panel.  In the fine model, the panel is
meshed with 10$\,$m square elements.

!media media/porous_flow/3D_coal_model.png
       id=3D.fig
       style=width:80%;margin-left:10px;
       caption=The geometry of the model and the mesh used.  A single longwall panel of total width
               300$\,$m and length 1000$\,$m is represented by the region with a fine mesh.

### Fluid mechanics

Single-phase unsaturated fluid is used with the nonlinear Variable being the fluid porepressure.
Full coupling with mechanical deformations is used, so the Kernels read

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i start=[mass0] end=[AuxVariables]

All stresses are measured in MPa and the time unit is years.  A Biot coefficient of 0.7 is used, and
the fluid properties are:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[Modules]
         end=[Materials]

A van Genuchten capillary pressure relationship is used

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[pc]
         end=[mc_coh_strong_harden]

A Corey relative permeability is used

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[relperm]
         end=[elasticity_tensor_0]

The fluid porepressure is fixed at the outer boundaries, and a
[PorousFlowPiecewiseLinearSink](/modules/porous_flow/src/bcs/PorousFlowPiecewiseLinearSink.C) with a
spatially and temporally varying `flux_function` is used to withdraw water from the rock into the
goaf:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[fix_porepressure]
         end=[roof_bcs]

In multi-phase scenarios, such a simple representation of the goaf boundary condition is not valid.


### Solid mechanics

Layered Cosserat solid mechanics is employed, meaning there are 3 translational and 2 rotational
degrees of freedom.  A quasi-static approximation is used, so the solid-mechanical Kernels are

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[cx_elastic]
         end=[mass0]

Quite a complicated elasto-plastic model is used and this accounts for the majority of nonlinear
iterations of this model, as well as the relative slowness of computing the residual and Jacobian.
The elasticity tensors (one for standard, one for Cosserat) are computed via:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[elasticity_tensor_0]
         end=[strain]

where the `excav_sideways` Function simulates the excavation of coal (it is 1 ahead of the coal face
and zero behind it).  Small strain is used, and an insitu stress that increases with depth is
assumed:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[strain]
         end=[stress_0]

Capped Mohr-Coulomb plus weak-plane plasticity is used and the plastic models are cycled

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[stress_0]
         end=[undrained_density_0]

The plastic moduli are:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[mc_coh_strong_harden]
         end=[Modules]

Roller boundary conditions are prescribed at the boundaries.  At the roof of the excavation, a
[StickyBC](/StickyBC.md) is employed to prevent the roof from collapsing
further than 3$\,$m:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[roof_bcs]
         end=[]

### Coupling

Full coupling between the fluid and solid mechanics is evident in the Kernels.  The
[PorousFlowDictator](/PorousFlowDictator.md) ensures all nonzero Jacobian entries are computed:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[dictator]
         end=[pc]

Other aspects of the fluid-solid coupling are the porosity:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i
         start=[porosity_bulk]
         end=[porosity_excav]

and the permeability:

!listing modules/porous_flow/examples/coal_mining/coarse_with_fluid.i start=[permeability_bulk] end=[permeability_excav]


## Results

Despite being a very simple model with no interesting lithology that usually induces characteristic
fracture and displacement patterns, the results display many interesting features.


!media porous_flow/destressed_evolution.gif
       id=destressed_evolution.fig
       style=width:80%;margin-left:10px;
       caption=The evolution of the destressed zone where the vertical stress has less magnitude
               than the initial vertical stress.  The zone is colored by the magnitude of
               de-stressing.  The black rectangle shows the coal that will be excavated.

!media porous_flow/displacement_evolution.gif
       id=displacement_evolution.fig
       style=width:80%;margin-left:10px;
       caption=The evolution of the zone where the vertical displacement is $<3\,$cm.  The zone is
               colored by the magnitude of vertical displacement.  The black rectangle shows the coal
               that will be excavated.

!media porous_flow/pp_evolution.gif
       id=pp_evolution.fig
       style=width:80%;margin-left:10px;
       caption=The evolution of the zone where the porepressure is reduced by more than $0.4\,$MPa.
               The zone is colored by the magnitude of the porepressure reduction.  The green arrows
               show the Darcy velocity.  The black rectangle shows the coal that will be excavated.

Some of interesting features are:

- Initially the roof holds up before suddenly collapsing at around 100m of excavation.

- After this point, there is some periodicity in the plastic strains and vertical stresses, although
  this may be an artificat of the mesh rather than anything physical.

- The minimum vertical displacement is actually $u_{z}=-3.3\,$m in the region where the sudden
  collapse occurs towards the start of the panel.  The StickyBC are not sufficient to prevent this.
  Therefore, the results towards the panel's beginning shouldn't be treated too seriously.  To avoid
  this, MOOSE's Constraint system, or the mining-specific features CSIRO's private MOOSE-based app,
  could be employed.

- There is significant rock fracture, indicated by the Mohr-Coulomb plastic strains of greater than
  $0.1$% in the immediate 10$\,$m of the roof.

- Mohr-Coulomb shear failure is much more prevelant than Mohr-Coulomb tensile failure throughout the
  model.

- Mohr-Coulomb shear failure also occurs in the unexcavated material close to the panel (the ribs and
  roadways).

- Tensile opening of the joints occurs in large regions of the roof, indicated by weak-plane tensile
  plasticity of greater than $0.1$%.

- Shear failure of the joints occurs throughout all regions of the model above the goaf.  It is
  greater than 1% in the immediate $\approx 100\,$m of the roof.

- The surrounding unexcavated material experiences large vertical loads, while there is an annular
  region on the outside and above the excavation that is de-stressed.  The material above the goaf
  therefore experiences de-stressing and then recompaction.

- Porosity and permeability enhancement typically occurs in regions of highest weak-plane failure.
  The permeability is increased by approximately a factor of 2 in the goaf region.  This is actually
  a tiny enhancement compared to the expected value of around $10^{5}$ and is due to the
  Kozeny-Carman permeability relationship which is not really valid in this setting.  If readers are
  interested in simulating realistic mining-induced permeability enhancements they should enquire
  about using CSIRO's private MOOSE-based app.

- Mining-induced drawdown of the water porepressure at the ground surface starts to occur after
  around $70\,$m (2 weeks) of excavation.  In this model, the region above the panel rapidly becomes
  unsaturated, with saturation reaching approximately 80%, as water flows to the goaf region.

- Most of the high Darcy fluid velocities occur around the periphery of the mining panel.

### Results along the centre-line of the panel.

The figures below show results contoured on a vertical section along the centre-line of the panel, after excavation has completed.  The excavation is shown as a black line.

!media media/porous_flow/long_mc_shear.png style=width:90%; caption=Mohr-Coulomb shear plastic strain on a vertical section along the centre-line of the panel

!media media/porous_flow/long_mc_tensile.png style=width:90%; caption=Mohr-Coulomb tensile plastic strain on a vertical section along the centre-line of the panel

!media media/porous_flow/long_wp_shear.png style=width:90%; caption=Weak-plane shear plastic strain on a vertical section along the centre-line of the panel

!media media/porous_flow/long_wp_tensile.png style=width:90%; caption=Weak-plane tensile plastic strain on a vertical section along the centre-line of the panel

!media media/porous_flow/long_perm_zz.png style=width:90%; caption=Vertical permeability on a vertical section along the centre-line of the panel

!media media/porous_flow/long_pp.png style=width:90%; caption=Porepressure (color) and Darcy flow vectors on a vertical section along the centre-line of the panel

### Results across the panel.

The figures below show results contoured on a vertical section across the half-panel that is modelled, after excavation has completed.  The excavation is shown as a black line.

!media media/porous_flow/cross_mc_shear.png style=width:90%; caption=Mohr-Coulomb shear plastic strain on a vertical section across the panel

!media media/porous_flow/cross_mc_tensile.png style=width:90%; caption=Mohr-Coulomb tensile plastic strain on a vertical section across the panel

!media media/porous_flow/cross_wp_shear.png style=width:90%; caption=Weak-plane shear plastic strain on a vertical section across the panel

!media media/porous_flow/cross_wp_tensile.png style=width:90%; caption=Weak-plane tensile plastic strain on a vertical section across the panel

!media media/porous_flow/cross_perm_zz.png style=width:90%; caption=Vertical permeability on a vertical section across the panel

!media media/porous_flow/cross_pp.png style=width:90%; caption=Porepressure (color) and Darcy flow vectors on a vertical section across the panel
