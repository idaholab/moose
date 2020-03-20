# XFEM Module

The XFEM module implements the extended finite element method (XFEM) in the MOOSE framework. It can
be used in conjuction with any number of other physics modules to enrich the solutions to partial
differential equations (PDEs) with some form of discontinuity. XFEM theory and algorithms employed
throughout this module are detailed in the following links:

- [Theory Manual](xfem/theory/theory.md)
- [Algorithms](xfem/theory/algorithms.md)

Tables provided in the [Objects, Actions, and Syntax](#objects-actions-and-syntax) section list
objects contained within the XFEM module followed by a short explanation of the object's purpose.
Links on the objects' names navigate to an individual detailed page for the object.

!media xfem/image21.gif
       id=crack
       style=width:100%;float:right;padding-top:2.7%;

!row!
!col small=12 medium=6 large=6
!media xfem/image79.gif
       caption=Side view of the deformation of a cylinder containing a cut prescribed by XFEM.

!col small=12 medium=6 large=6
!media xfem/image78.gif
       caption=Top view of the deformation of a cylinder containing a cut prescribed by XFEM.

!row-end!

## Applications

XFEM is ideally suited to problems involving local discontinuities. Typically these types of
problems fall into one of two categories: strong or weak discontinuities. An example of a strong
discontinuity is a physical break in the domain of the problem (e.g. cracks), while a weak
discontinuity deals with discontinuous properties or solutions (e.g. discrete material regions).
Examples of the types of problems XFEM can solve follow:

- Cracking

  - Prescribed crack growth
  - Cracks propagating based on solution values (stress, pressure, etc.)

- Moving Interfaces

  - Dynamic material interfaces
  - Open (glued) or closed (inclusion) interfaces on the domain
  - Phase transition (Stefan) problems

To support interfaces that move based upon calculated growth rates rather than prescribed functions,
coupling with the [level set module](level_set/index.md optional=True) is recommended.

## Software Quality

The XFEM module follows strict software quality guidelines. Please refer to
[XFEM Software Quality Assurance](xfem/sqa/index.md) for additional information.

## Objects, Actions, and Syntax

!syntax complete groups=XFEMApp level=3
