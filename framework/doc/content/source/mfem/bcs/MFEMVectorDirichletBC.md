# MFEMVectorDirichletBC

## Summary

!syntax description /BCs/MFEMVectorDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on all components of a
vector $H^1$ conforming variable on the boundary. The boundary value is constant in space and time.

## Example Input File Syntax

!listing test/tests/kernels/linearelasticity.i block=BCs

!syntax parameters /BCs/MFEMVectorDirichletBC

!syntax inputs /BCs/MFEMVectorDirichletBC

!syntax children /BCs/MFEMVectorDirichletBC
