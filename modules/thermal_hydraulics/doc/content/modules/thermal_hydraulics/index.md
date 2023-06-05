!config navigation breadcrumbs=False scrollspy=False

# Thermal Hydraulics Module class=center style=font-size:300%

## The MOOSE-based framework for thermal hydraulics simulations class=center style=font-weight:200;font-size:175%


!row!
!col! small=12 medium=7 large=7 icon=storage

### Features class=center style=font-weight:400

The basic capabilities include:

- Component-based approach to build thermal-hydraulic simulations.
- 1-phase, variable-area, inviscid, compressible flow model.

  - with non-condensable vapor mixture.

- 2-D (Cartesian or axisymmetric) heat conduction.
- 3-D heat conduction.
- Control logic system.
- Extensibility:

  - pluggable closure system
  - pluggable models

- and much more...

!col-end!

!col! small=12 medium=5 large=5 icon=school

### Academy class=center style=font-weight:400

Get started running your own THM simulations by exploring the introductory tutorial.
Use the modeling guide to explore how to piece your models together:

- [Getting Started](modules/thermal_hydraulics/getting_started.md) -- how to install the code.
- Tutorials

  - [Basics](tutorials/basics/index.md) -- learn the basics.
  - [Single-phase flow](tutorials/single_phase_flow/index.md) -- learn about single-phase flow.

- [Modeling Guide](modeling_guide/index.md) -- learn how to build your models.
- [Verification and Validation](v_and_v/index.md) -- verification and validation tests performed to date.
- [Syntax Index](syntax/index.md) -- complete syntax listing.

!col-end!

!row-end!

## Announcements and News

#### April 20, 2022

The [Thermal hydraulics module](modules/thermal_hydraulics/index.md) can now run with distributed memory parallelism (MPI).
Note that [HeatTransferFromHeatStructure1Phase.md] currently requires a replicated mesh; see [issue](https://github.com/idaholab/moose/issues/20798).

[Benchmarks](https://thm-benchmarks.readthedocs.io/en/latest/index.html) are available.


#### December 7, 2021

Closures objects are now created in the input file, allowing for greater flexibility,
since some closures classes could take parameters for customization, which was
not possible in the old setup. For example, the closures option `simple` corresponds
to the class [Closures1PhaseSimple.md], so the new setup creates a closures object
of this class in the input file:

```
[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]
```

Then the user-given name (in this case, `simple_closures`) is provided to the
`closures` parameter instead of `simple`.

!alert warning
+Warning:+ The old setup is now deprecated and will be removed March 31, 2022.

####  September 1, 2021

THM was converted to use the automated differentiation (AD) system available in MOOSE.
While this brings many improvements, it also breaks input files using the 1-phase flow formulation.
Please use the [migration guide](howto/thm_ad_migration_guide.md) to update your input files.


### Developed by class=center style=font-size:90%;margin-bottom:0.5em;font-weight:100

!media large_media/framework/inl_blue.png style=width:20%;display:block;margin-left:auto;margin-right:auto;
