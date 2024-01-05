# ElementHCurlError

!syntax description /Postprocessors/ElementHCurlError

## Overview

The H(curl)-error is the square root of the sum of the squares of the
[L2-error](ElementVectorL2Error.md) and the
[H(curl)-semierror](ElementHCurlSemiError.md).
All input parameters and their restrictions are the same as those for
[ElementHCurlSemiError.md].

## Example Input File Syntax

!listing vector_kernel.i block=Postprocessors/HCurlError

!syntax parameters /Postprocessors/ElementHCurlError

!syntax inputs /Postprocessors/ElementHCurlError

!syntax children /Postprocessors/ElementHCurlError