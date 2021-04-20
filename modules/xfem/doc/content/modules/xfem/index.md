# XFEM Module

!row!
!col! small=12 medium=6 large=9
The XFEM module implements the extended finite element method (XFEM) in the MOOSE framework. It can
be used in conjuction with any number of other physics modules to enrich the solutions to partial
differential equations (PDEs) with some form of discontinuity. XFEM theory and implementation are
detailed in the theory manual:

- [XFEM Theory Manual](xfem/theory/theory.md)
- [Embedded Interface](xfem/theory/embedded_interface.md)

Tables provided in the [Objects, Actions, and Syntax](#objects-actions-and-syntax) section list
objects contained within the XFEM module followed by short explanations of the objects' purposes.
Links on the objects' names navigate to an individual detailed page for the object.

!media xfem/image21.gif
       style=width:100%;float:right;padding-top:2.7%;
       caption=Crack propagation on a circular mesh as a result of hoop strain relief.

!col-end!

!col! small=12 medium=6 large=3
!media xfem/image79.gif
       caption=Side view of the deformation of a cylinder with a cut prescribed by XFEM.

!media xfem/image78.gif
       caption=Top view of the deformation of a cylinder with a cut prescribed by XFEM.
!col-end!

!row-end!

## Applications

XFEM is ideally suited to problems involving local discontinuities. Typically these types of
problems fall into one of two categories: strong or weak discontinuities. Strong discontinuities
are characterized by a jump in the value of a solution field across an interface, whereas weak
discontinuities involve a jump in the first derivative (slope) of a solution field across an
interface. Examples of the types of problems XFEM can solve include:

- Cracking

  - Stationary cracks
  - Cracks that propagate in a manner directly prescribed by the user
  - Cracks that propagate based on the solution (stress, stress intensity factor, etc.)

- Interfaces

  - Open (glued) or closed (inclusion) interfaces on the domain
  - Static and dynamic movement options

    - Stationary (prescribed location) interfaces
    - Interfaces that move in a manner directly prescribed by the user (provided level set function)
    - Interfaces that move based on the solution (temperature, displacement, etc.)

The locations of the interface used to define solution discontinuities can be prescribed in various
ways, including by user-defined cutting planes and using level set functions. The evolution of level
set functions can optionally be computed using the
[level set module](level_set/index.md optional=True), which is recommended for interfaces that move
based on calculated growth rates rather than prescribed functions.

## Software Quality

The XFEM module follows strict software quality guidelines. Please refer to
[XFEM Software Quality Assurance](xfem/sqa/index.md) for additional information.

## Objects, Actions, and Syntax

!syntax complete groups=XFEMApp level=3
