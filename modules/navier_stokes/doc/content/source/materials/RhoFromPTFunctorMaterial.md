# RhoFromPTFunctorMaterial

!syntax description /Materials/RhoFromPTFunctorMaterial

## Overview

This object takes a fluid properties object and coupled pressure and temperature
variables and provides a functor material property rho that is computed through
a call to `_fluid.rho_from_p_T(_pressure(x), _temperature(x))` where `x`
represents a physical location, e.g. on an element for a face.

!syntax parameters /Materials/RhoFromPTFunctorMaterial

!syntax inputs /Materials/RhoFromPTFunctorMaterial

!syntax children /Materials/RhoFromPTFunctorMaterial
