# Using Stochastic Tools for the full-field reconstruction of multiphysics problems

The purpose of this example is to present a multiphysics model order reduction case using
the [Stochastic Tools Module](modules/stochastic_tools/index.md). The problem of interest is a
melt pool model using the [Navier-Stokes](modules/navier_stokes/index.md) module.
The problem involves multiple uncertain process parameters and are interested in predicting
full field quantities for unseen process parameter combinations using non-intrusive surrogate
models in combination with Proper Orthogonal Decomposition (POD).

## Problem Description

The problem of interest involves a transient 2D melt pool simulation for a 316L stainless steel
with a moving Gaussian laser beam. The laser source melts the top layer of the metal and
based on the equlibrium of the forces (surface tension, vapor recoil pressure) the surface of the
melt pool can deform considerably. The changing geometry is taken into account using an
Arbitrary Legrangian Eulerian (ALE) approach. The setup of the problem is depicted in [fig:geom].

!media combined/geom.png caption=Problem setup for the laser melt pool simulation id=fig:geom style=width:50%

The temperature-dependent thermal properties of the problem have been taken from the
references listed in [tab:mat_prop].

!table caption=Material properties for 316L Stailess steel id=tab:mat_prop
| Property | Source|
| :- | :- |
| Dynamic viscosity | [!cite](kim1975thermophysical)$^\dagger$ |
| Density | [!cite](kim1975thermophysical) |
| Specific heat | [!cite](kim1975thermophysical) |
| Thermal conductivity | [!cite](pichler2022surface) |
| Surface tension | [!cite](pichler2022surface) |
| Vapor recoil pressure | [!cite](chen2021numerical) |

${\dagger} in the treatment of dynamic viscosity, following the recommendations in [!cite](noble2007use),
we artificially cap the minimum value to stabilize the fluid flow solution algorithms.

The input file used for the simulations is presented below:

!listing examples/stochastic/laser_welding/2d.i caption=Laser meltpool model input file id=list:meltpool

We see that it utilizes the `include` syntax to read the process and geometry parameters are
in the subsequent sections.

### Process parameters

A total of two process parameters have been identified to be changable: the effective laser power and
the effective laser radius. They can change in the intervals shown in [tab:ppars] assuming a uniform
distribution.

!table caption=The distributions used for the process parameters id=tab:uprop
| Property | Min. | Max. |
| :- | - | - |
| Effective laser power  | 60 W | 74 W |
| Effective laser radius | 125 $\mu m$ | 155 $\mu m$$\mu m$ |

These parameters are defined in the following input file:

!listing examples/stochastic/laser_welding/parameters.i caption=Summary of process and geometry parameters for the 2D melt pool simulation id=list:ppars

### Quantities of Interest

The quantity of interest in our case is the whole temperature field at the end of the simulation (0.4~ms).
To be able to reconstruct the whole field we utilize Proper Orthogonal Decomposition (POD) in conjunction
 with a Multi-output Gaussian Process
(MOGP).

## Results

## Reference results

Two reference results are presented here:
1. Results for a simulation with low effective laser power ($60~W$) and wide effective laser radius ($155~\mu m$)
2. Results for a simulation with high effective laser power ($74~W$) and narrow effective laser radius ($125~\mu m$)

The reference results are filtered to show the melt pool shape. We see that for the first case, the
steel melts, but no displacement is visible due to little to no evaporation. On the other hand, the
increase in laser power results in an increased evaporation resulting in significant surface deformation.

## Training the reduced-order model

The reduced-order model was trained by first running the reference model with X different
process parameter combinations in a monte carlo sampler. The input file used for training
is presented below.

!listing examples/stochastic/laser_welding/2d.i caption=Laser meltpool model input file id=list:training

The parts that require special attention are the snapshot gathering
parts in the transfer and reporter blocks.

!listing examples/stochastic/laser_welding/2d.i caption=Laser meltpool model input file id=list:training

With the snapshots all available in one place, we can extract the POD modes
necessary for the dimensionality reduction. This is dones uesing the
[PODMapping.md] reporter which not only creates the decomposition but also maps the
created snapshots onto a low-dimensional latent space. In this specific example, the
dimensionality of the latent space is selected to be X.
Once the coordinates of the snapshots are available in the latent space, we can fit a
MOGP on them. This is done in the `Trainers` block.

!listing examples/stochastic/laser_welding/2d.i caption=Laser meltpool model input file id=list:training

One can then save both the POD mapping and MOGP into restratable structures for
later usage.

!listing examples/stochastic/laser_welding/2d.i caption=Laser meltpool model input file id=list:training

## Evaluating the error of the ROM


