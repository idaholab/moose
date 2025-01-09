# MFEMVectorFunctorNormalDirichletBC

## Summary

!syntax description /BCs/MFEMVectorFunctorNormalDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the normal
components of a $H(\mathrm{div})$ conforming vector FE at a boundary. The imposed value is
a function of space and/or time.

## Example Input File Syntax

!listing test/tests/kernels/graddiv.i block=BCs

!syntax parameters /BCs/MFEMVectorFunctorNormalDirichletBC

!syntax inputs /BCs/MFEMVectorFunctorNormalDirichletBC

!syntax children /BCs/MFEMVectorFunctorNormalDirichletBC
