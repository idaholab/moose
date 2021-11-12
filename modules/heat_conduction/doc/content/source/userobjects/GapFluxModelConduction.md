# GapFluxModelConduction

!syntax description /UserObjects/GapFluxModelConduction

## Description

`GapFluxModelConduction` computes a conductive heat flux across a gap following the existing implementation of radiation physics. It is used by
[ModularGapConductanceConstraint.md](ModularGapConductanceConstraint.md).

The user is required to select the appropriate `gap_geometry_type` parameter (PLATE, CYLINDER, or SPHERE) for the model geometry in ModularGapConductanceConstraint.md](ModularGapConductanceConstraint.md). Two-dimensional Cartesian geometries are not restricted to be in or parallel to the X-Y coordinate plane.


!syntax parameters /UserObjects/GapFluxModelConduction

!syntax inputs /UserObjects/GapFluxModelConduction

!syntax children /UserObjects/GapFluxModelConduction

!bibtex bibliography
