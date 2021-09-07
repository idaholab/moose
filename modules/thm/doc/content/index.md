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

- [Getting Started](getting_started.md) -- how to install the code.
- Tutorials

  - [Basics](tutorials/basics/index.md) -- learn the basics.
  - [Single-phase flow](tutorials/single_phase_flow/index.md) -- learn about single-phase flow.

- [Modeling Guide](modeling_guide/index.md) -- learn how to build your models.
- [Verification and Validation](v_and_v/index.md) -- verification and validation tests performed to date.
- [Syntax Index](syntax/index.md) -- complete syntax listing.

!col-end!

!row-end!

## Announcements and News

####  September 1, 2021

THM was converted to use the automated differentiation (AD) system available in MOOSE.
While this brings many improvements, it also breaks input files using the 1-phase flow formulation.
Please use the [migration guide](howto/thm_ad_migration_guide.md) to update your input files.


### Developed by class=center style=font-size:90%;margin-bottom:0.5em;font-weight:100

!media media/inl_blue.png style=width:20%;display:block;margin-left:auto;margin-right:auto;
