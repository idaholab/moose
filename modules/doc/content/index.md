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
!card! gallery/corner_flow.gif title=Porous Flow around Corner style=height:320px
Transient flow around corner using porous flow approximation of Euler equations with variable porosity,
see [Pronghorn: Porous media thermal-hydraulics for reactor applications](https://escholarship.org/uc/item/61k9r05w).
!card-end!

!card! gallery/elder.gif title=Density driven, porous flow with adaptivity style=height:320px
Density driven, porous flow simulation of the Elder problem using [Falcon](https://github.com/idaholab/falcon) (*credit: Robert Podgorney (INL)*).
!card-end!

!card! gallery/step10_result.gif title=Multi-scale Simulation style=height:300px
Engineering scale porous flow, modeled using Darcy's equation within a cylinder assuming a porous
media of closely packed steel spheres, see [MultiApps/index.md].
!card-end!

!card! gallery/densification.gif title=3D Densification of snow style=height:300px
Densification of a 3D snow pack using empirical, density based continuum model using [Pika](https://github.com/idaholab/pika).
!card-end!
!gallery-end!

!gallery! large=6
!card! gallery/snow.gif title=Dendritic Crystal Growth style=height:480px
Process of dendritic crystal growth, which is an anisotropic nucleation process as presented by
[Modeling and numerical simulations of dendritic crystal growth](https://www.sciencedirect.com/science/article/pii/016727899390120P).

!style fontsize=80% fontweight=100
*Credit*: Yang Xia, Department of Material Science and Engineering,
Shanghai Jiao Tong University, Shanghai.
!card-end!

!card! gallery/ch_40.gif title=3D spinodal decomposition style=height:480px
A 3D spinodal decomposition modeled with Cahn-Hilliard equations using third-order Hermite elements with
the [phase field module](modules/phase_field/index.md).
!card-end!

!card! gallery/grain_tracker.gif title=Casey Here style=height:380px
This is a place holder for Casey.
!card-end!

!card! gallery/grain_tracker.gif title=3D Grain Tracking style=height:400px;
The [GrainTracker](GrainTracker.md) is a utility that dramatically reduces the number of order
parameters needed to model a large polycrystal system with the phase-field module. This video shows
the dynamic remapping that occurs as "reused" order parameters get too close to one and other as the
simulation evolves.
!card-end!

!card! level_set/vortex_out.gif title=Vortex Benchmark style=height:460px;
The level set equation is commonly used to for interface tracking, especially when the interface
velocity is known. MOOSE contains a level set module, for more information see [level_set/index.md].
!card-end!

!card! gallery/soil.gif title=Soil desiccation simulation style=height:460px;
A 3D soil desiccation simulation using phase-field for cohesive fracture model, see
[A phase-field formulation for dynamic cohesive fracture](https://arxiv.org/abs/1809.09691) and
[Duke Computational Mechanics Lab](http://dcml.pratt.duke.edu/).
!card-end!
!gallery-end!

!media media/inl_blue.png style=width:30%;display:block;margin-top:3em;margin-left:auto;margin-right:auto;
