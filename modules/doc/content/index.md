!config navigation breadcrumbs=False scrollspy=False long-name=Parallel Multiphysics Finite Element Framework

# HOME style=visibility:hidden;

!media media/moose_logo.png style=display:block;margin-left:auto;margin-right:auto;width:60%;

# Multiphysics Object Oriented Simulation Environment class=center light style=font-size:300%

# An open source, parallel finite element framework class=center style=font-weight:200;font-size:200%

!row!
!col! small=12 medium=4 large=4 icon=flash_on
### Rapid Development class=center style=font-weight:200;

MOOSE provides a plug-in infrastructure that simplifies definitions of physics, material properties,
and postprocessing.
!col-end!

!col! small=12 medium=4 large=4 icon=group
### User Focused class=center style=font-weight:200;

MOOSE includes ever-expanding set of [physics modules](modules/index.md) and supports multi-scale models, enabling
collaboration across applications, time-scales, and spatial domains.
!col-end!

!col! small=12 medium=4 large=4 icon=settings
### Getting Started class=center style=font-weight:200;

MOOSE works on Mac OS, Linux, and Windows and is easy to get started with. Begin by exploring
[our getting started page](getting_started/index.md).
!col-end!
!row-end!


!gallery! large=6

!card! gallery/golem_app_reservoir_analysis.gif title=Faulted Geothermal Reservoirs
Evolution of the 100C temperature during 30 years of injection/production for a complex doublet
system into a fractured geothermal reservoir. Based on [GOLEM](https://github.com/ajacquey/golem),
a MOOSE based application for thermo-hydro-mechanical simulations of fractured reservoirs.

!style fontsize=90% fontweight=100
Cacace M., Jacquey, A.B. (2017): [Flexible parallel implicit modeling of coupled thermal-hydraulic-mechanical processes in fractured rocks.](https://www.solid-earth.net/8/921/2017/) Solid Earth
!card-end!

!card! gallery/laser_welding.gif title=Laser Melt Pool
Using an arbitrary Lagrangian-Eulerian (ALE) formulation a laser is rotated around the surface of a stainless
steel block. The steel first melts and then begins to evaporate. The recoil force from evaporation
deforms the surface of the melt pool which in turn drives flow in the melt pool interior. Melt flow
is determined using the incompressible [Navier-Stokes](modules/navier_stokes/index.md) equations
while mesh deformation is determined using a linear elasticity equation.
!card-end!

!card! gallery/corner_flow.gif title=Single-phase Flow in a Packed Bed
Transient flow around corner using Euler equations with variable porosity,
see [Pronghorn: Porous media thermal-hydraulics for reactor applications](https://escholarship.org/uc/item/61k9r05w).
!card-end!

!card! gallery/elder.gif title=Density Driven, Porous Flow with Adaptivity
Density driven, porous flow simulation of the Elder problem using [Falcon](https://github.com/idaholab/falcon).  Mesh adaptivity is used to accurately capture the moving fronts.

!style fontsize=80% fontweight=100
*Credit:* Robert Podgorney (INL)
!card-end!

!card! gallery/step10_result.gif title=Multi-scale Simulation
Engineering scale porous flow, modeled using Darcy's equation within a cylinder assuming a porous
media of closely packed steel spheres, see [MultiApps/index.md].
!card-end!

!card! gallery/densification.gif title=3D Densification of Snow
Densification of a 3D snow pack using empirical, density based continuum model using [Pika](https://github.com/idaholab/pika).
!card-end!

!card! gallery/snow.gif title=Dendritic Crystal Growth
Process of dendritic crystal growth, which is an anisotropic nucleation process as presented by
[Modeling and numerical simulations of dendritic crystal growth](https://www.sciencedirect.com/science/article/pii/016727899390120P).

!style fontsize=80% fontweight=100
*Credit:* Yang Xia, Department of Material Science and Engineering,
Shanghai Jiao Tong University, Shanghai.
!card-end!

!card! gallery/ch_40.gif title=3D Spinodal Decomposition
A 3D spinodal decomposition modeled with Cahn-Hilliard equations using third-order Hermite elements with
the [phase field module](modules/phase_field/index.md).
!card-end!

!card! gallery/dipole_antenna.gif title=2D Half-Wave Dipole Antenna
The 2D electric field radiation pattern of a broadcasting half-wave dipole antenna, modeled using
the [VectorMooseVariable.md] system with first-order Nedelec elements.
!card-end!

!card! gallery/grain_tracker.gif title=3D Grain Tracking
The [GrainTracker](GrainTracker.md) is a utility that dramatically reduces the number of order
parameters needed to model a large polycrystal system with the phase-field module. This video shows
the dynamic remapping that occurs as "reused" order parameters get too close to one and other as the
simulation evolves.
!card-end!

!card! level_set/vortex_out.gif title=Vortex Benchmark
The level set equation is commonly used to for interface tracking, especially when the interface
velocity is known. MOOSE contains a level set module, for more information see [level_set/index.md].
!card-end!

!card! gallery/soil.gif title=Soil Desiccation Simulation
A 3D soil desiccation simulation using phase-field for cohesive fracture model, see
[A phase-field formulation for dynamic cohesive fracture](https://arxiv.org/abs/1809.09691).

!style fontsize=80% fontweight=100
*Credit:* Gary Hu, [Duke Computational Mechanics Lab](http://dcml.pratt.duke.edu/)
!card-end!
!gallery-end!

# Select MOOSE Features class=center style=font-weight:200;

- Flexible Plug-In Architecture Reducing Code Development
- [Automatic Differentiation](NonlinearSystem.md)
- Continuous Finite Element
- Discontinuous Finite Element
- Mixed (CG and DG, coupled in the same simulation)
- Massively Parallel: Hybrid MPI + Threading / OpenMP
- [Scalability Proven to Over 30,000 cores](https://ieeexplore.ieee.org/abstract/document/8638143)
- Massive Problem Sizes (1B+ elements, 100B+ unknowns)
- [Multiscale Solves](MultiApps/index.md)
- Leverages [PETSc](https://www.mcs.anl.gov/petsc) solvers
- [Physics Modules:](modules/index.md)

  - [Heat Conduction](HeatConduction.md)
  - [Navier Stokes](NavierStokes/index.md)
  - [Solid Mechanics](syntax/Modules/TensorMechanics/index.md)
  - [Contact](contact/index.md)
  - [Porous Flow](porous_flow/index.md)
  - [Phase Field](phase_field/index.md)
  - [Level Set](level_set/index.md)

- Unstructured Mesh (Quads, Tris, Hexes, Tets, Pyramids, Wedges, etc.)
- Curvilinear Geometry
- [Mesh Adaptivity](syntax/Adaptivity/index.md)
- [In-situ Postprocessing](Postprocessors/index.md)
- [Nonlinear Material Properties](examples/ex08_materials.md)
- [Physics-Based Damping](Dampers/index.md)
- Point Sources
- Multiple Formats for Input and Output ([Exodus](Exodus.md), [VTK](VTKOutput.md), GMSH, etc.)
- Nonlinear, Coupled ODE / PDE Systems
- Arbitrary Lagrangian-Eulerian (ALE) formulation

!media media/inl_blue.png style=width:30%;display:block;margin-top:3em;margin-left:auto;margin-right:auto;
