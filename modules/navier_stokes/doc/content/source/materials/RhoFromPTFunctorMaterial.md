# RhoFromPTFunctorMaterial

!syntax description /Materials/RhoFromPTFunctorMaterial

## Overview

This object takes a fluid properties object and coupled pressure and temperature
variables and provides a functor material property `rho` that is evaluated with
the local pressure and temperature as `rho_from_p_T(_pressure(x), _temperature(x))` where `x`
represents a physical location, e.g. on an element or a face.

!syntax parameters /Materials/RhoFromPTFunctorMaterial

!syntax inputs /Materials/RhoFromPTFunctorMaterial

!syntax children /Materials/RhoFromPTFunctorMaterial
