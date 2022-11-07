!gallery! large=6

!card! gallery/3D_SSB.mp4 title=Charge-discharge cycle of a full solid-state battery
A complete charge-discharge cycle of a 3D full solid-state battery. The results are generated using EEL developed by the Argonne National Laboratory. The theoretical framework is described in [Hu, T., Messner, M. C., Barai, P., & Barua, B. (2022)](https://doi.org/10.2172/1891097).

!style fontsize=90% fontweight=100
*Credit:*  Gary Hu (ANL)
!card-end!

!card! gallery/streamlines_rock.mp4 title=Flow Streamlines in Digital Rock
Flow streamlines computed with the [Navier-Stokes](modules/navier_stokes/index.md) module, through a digital rock reconstructed with the [Image Reader](source/functions/ImageFunction.md) from a stack of microCT scans (postprocessing done in [Paraview](https://www.paraview.org/)). The framework is described in [Lesueur et al. (2017)](http://doi.org/10.1016/j.gete.2017.08.001).

!style fontsize=90% fontweight=100
*Credit:*  Martin Lesueur, [Multiphysics Geomechanics Lab](https://mglab.pratt.duke.edu/)
!card-end!

!card! gallery/twist_gallery.mp4 title=Wire Twist
Mechanical modeling of two copper wires being twisted together to create a twisted pair
using the [Tensor Mechanics](modules/tensor_mechanics/index.md) and [Contact](modules/contact/index.md) modules.

!style fontsize=90% fontweight=100
*Credit:*  [Materials Science and Scientific Computing Department at UKAEA](https://ccfe.ukaea.uk/research/)
!card-end!

!card! gallery/golem_app_reservoir_analysis.mp4 title=Faulted Geothermal Reservoirs
Evolution of the 100C temperature during 30 years of injection/production for a complex doublet
system into a fractured geothermal reservoir. Based on [GOLEM](https://github.com/ajacquey/golem),
a MOOSE based application for thermo-hydro-mechanical simulations of fractured reservoirs.

!style fontsize=90% fontweight=100
Cacace M., Jacquey, A.B. (2017): [Flexible parallel implicit modeling of coupled thermal-hydraulic-mechanical processes in fractured rocks.](https://www.solid-earth.net/8/921/2017/) Solid Earth
!card-end!

!card! gallery/laser_welding.mp4 title=Laser Melt Pool
Using an arbitrary Lagrangian-Eulerian (ALE) formulation a laser is rotated around the surface of a stainless
steel block. The steel first melts and then begins to evaporate. The recoil force from evaporation
deforms the surface of the melt pool which in turn drives flow in the melt pool interior. Melt flow
is determined using the incompressible [Navier-Stokes](modules/navier_stokes/index.md) equations
while mesh deformation is determined using a linear elasticity equation.
!card-end!

!card! gallery/corner_flow.mp4 title=Single-phase Flow in a Packed Bed
Transient flow around corner using Euler equations with variable porosity,
see [Pronghorn: Porous media thermal-hydraulics for reactor applications](https://escholarship.org/uc/item/61k9r05w).
!card-end!

!card! gallery/elder.mp4 title=Density Driven Porous Flow
Density driven, porous flow simulation of the Elder problem using [Falcon](https://github.com/idaholab/falcon).  Mesh adaptivity is used to accurately capture the moving fronts.

!style fontsize=80% fontweight=100
*Credit:* Robert Podgorney (INL)
!card-end!

!card! gallery/step10_result.mp4 title=Multi-scale Simulation
Engineering scale porous flow, modeled using Darcy's equation within a cylinder assuming a porous
media of closely packed steel spheres, see [MultiApps/index.md].
!card-end!

!card! gallery/densification.mp4 title=3D Densification of Snow
Densification of a 3D snow pack using empirical, density based continuum model using [Pika](https://github.com/idaholab/pika).
!card-end!

!card! gallery/snow.mp4 title=Dendritic Crystal Growth
Process of dendritic crystal growth, which is an anisotropic nucleation process as presented by
[Modeling and numerical simulations of dendritic crystal growth](https://www.sciencedirect.com/science/article/pii/016727899390120P).

!style fontsize=80% fontweight=100
*Credit:* Yang Xia, Department of Material Science and Engineering,
Shanghai Jiao Tong University, Shanghai.
!card-end!

!card! gallery/ch_40.mp4 title=3D Spinodal Decomposition
A 3D spinodal decomposition modeled with Cahn-Hilliard equations using third-order Hermite elements with
the [phase field module](modules/phase_field/index.md).
!card-end!

!card! gallery/dipole_antenna.mp4 title=2D Half-Wave Dipole Antenna
The 2D electric field radiation pattern of a broadcasting half-wave dipole antenna, modeled using
the [modules/electromagnetics/index.md] with first-order Nedelec elements.
!card-end!

!card! gallery/grain_tracker.mp4 title=3D Grain Tracking
The [GrainTracker](GrainTracker.md) is a utility that dramatically reduces the number of order
parameters needed to model a large polycrystal system with the phase-field module. This video shows
the dynamic remapping that occurs as "reused" order parameters get too close to one and other as the
simulation evolves.
!card-end!

!card! level_set/vortex_out.mp4 title=Vortex Benchmark
The level set equation is commonly used to for interface tracking, especially when the interface
velocity is known. MOOSE contains a level set module, for more information see [level_set/index.md].
!card-end!

!card! gallery/soil.mp4 title=Soil Desiccation Simulation
A 3D soil desiccation simulation using phase-field for cohesive fracture model, see
[A phase-field model of fracture with frictionless contact and random fracture properties: Application to thin-film fracture and soil desiccation](https://doi.org/10.1016/j.cma.2020.113106).

!style fontsize=80% fontweight=100
*Credit:* Gary Hu, [Dolbow Research Group](http://dolbow.pratt.duke.edu/)
!card-end!

!card! contact/2d_indenter.mp4 title=Axisymmetric Spherical Indenter
An elastic spherical indenter penetrates into a base material modeled with tensor-mechanics crystal plasticity.
[contact/index.md].
!card-end!

!card! contact/ironing_gallery.mp4 title=Frictional Ironing Problem with Mortar Contact
A deformable semi-circular tool pushes into highly deformable material generating tangential deformation.
[contact/index.md].
!card-end!
!gallery-end!
