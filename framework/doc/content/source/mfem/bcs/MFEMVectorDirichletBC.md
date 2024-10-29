# MFEMVectorDirichletBC

## Summary

!syntax description /BCs/MFEMVectorDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on a vector variable on the
boundary.

The components of the vector variable on which the essential boundary condition are applied are
controlled by the `_boundary_apply_type` parameter. Currently, only the tangential components of the
vector variable on the boundary are fixed to the values of the input vector coefficient.

## Example Input File Syntax

!listing test/tests/kernels/curlcurl.i block=BCs

!syntax parameters /BCs/MFEMVectorDirichletBC

!syntax inputs /BCs/MFEMVectorDirichletBC

!syntax children /BCs/MFEMVectorDirichletBC
