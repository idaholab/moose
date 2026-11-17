# MFEMNormPostprocessor

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM postprocessors computing a norm over the mesh.

## Overview

`MFEMNormPostprocessor` is the base class of [MFEMPostprocessor.md] objects computing a norm by
quadrature over the elements of the mesh, such as the norm of the error of a variable against a
function.

The `quadrature_order` parameter selects the quadrature rule of that order on every element.
Without it, each element uses a rule of order $2p+3$, where $p$ is the polynomial order of the
variable's element on it. A lower order can underestimate the error considerably, because the
error of a finite element solution is often smallest at the points of a Gauss rule.

!if-end!

!else
!include mfem/mfem_warning.md
