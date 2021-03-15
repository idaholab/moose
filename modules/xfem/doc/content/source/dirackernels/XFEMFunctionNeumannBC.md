# XFEMFunctionNeumannBC

!syntax description /DiracKernels/XFEMFunctionNeumannBC

## Overview

`XFEMFunctionNeumannBC` applies a Neumann BC on both sides of an XFEM interface. The value of the BC is provided by a `Function`.

!alert note
An XFEM interface can either be a material interface or a crack surface. This object works in both scenarios.

## Example Input Syntax

!listing test/tests/neumann_bc/2d_function.i block=DiracKernels

!syntax parameters /DiracKernels/XFEMFunctionNeumannBC

!syntax inputs /DiracKernels/XFEMFunctionNeumannBC

!syntax children /DiracKernels/XFEMFunctionNeumannBC
