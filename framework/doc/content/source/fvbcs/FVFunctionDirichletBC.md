# FVFunctionDirichletBC

!syntax description /FVBCs/FVFunctionDirichletBC

## Overview

`FVFunctionDirichletBC` is very similar to [/FVDirichletBC.md] except the
parameter `value` is replaced by the `function` parameter, where the latter is a
`FunctionName` or alternatively a direct input of a parsed
function. `FVFunctionDirichletBC` is generally useful; it's critical for
implementing MMS studies.

## Example Input File Syntax

!listing fvkernels/mms/advection-diffusion.i block=FVBCs

!syntax parameters /FVBCs/FVFunctionDirichletBC

!syntax inputs /FVBCs/FVFunctionDirichletBC

!syntax children /FVBCs/FVFunctionDirichletBC
