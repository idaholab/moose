# ElementHDivError

!syntax description /Postprocessors/ElementHDivError

## Overview

The H(div)-error is the square root of the sum of the squares of the
[L2-error](ElementVectorL2Error.md) and the
[H(div)-semierror](ElementHDivSemiError.md).
All input parameters and their restrictions are the same as those for
[ElementHDivSemiError.md].

## Example Input File Syntax

!listing coupled_electrostatics.i block=Postprocessors/HDivError

!syntax parameters /Postprocessors/ElementHDivError

!syntax inputs /Postprocessors/ElementHDivError

!syntax children /Postprocessors/ElementHDivError