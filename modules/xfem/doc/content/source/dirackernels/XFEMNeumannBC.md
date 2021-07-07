# XFEMNeumannBC

!syntax description /DiracKernels/XFEMNeumannBC

## Overview

`XFEMNeumannBC` applies a constant Neumann BC on both sides of an XFEM interface.

!alert note
An XFEM interface can either be a material interface or a crack surface. This object works in both scenarios.

## Example Input Syntax

!listing test/tests/neumann_bc/2d_constant.i block=DiracKernels

!syntax parameters /DiracKernels/XFEMNeumannBC

!syntax inputs /DiracKernels/XFEMNeumannBC

!syntax children /DiracKernels/XFEMNeumannBC
