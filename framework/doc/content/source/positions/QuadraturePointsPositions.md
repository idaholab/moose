# QuadraturePointsPositions

!syntax description /Positions/QuadraturePointsPositions

## Overview

The `QuadraturePointsPositions` may be block-restricted to limit the gathering of quadrature points to elements
from certain subdomains.

This object is currently set to:

- using the default element order for the quadrature
- using the Gauss quadrature rule
- using the element quadrature


!alert note
The `QuadraturePointsPositions`, when used in conjunction with a [TransientMultiApp.md]
enables creating the same subapps as the [QuadraturePointMultiApp.md], except that it is not limited to transients.

!syntax parameters /Positions/QuadraturePointsPositions

!syntax inputs /Positions/QuadraturePointsPositions

!syntax children /Positions/QuadraturePointsPositions
