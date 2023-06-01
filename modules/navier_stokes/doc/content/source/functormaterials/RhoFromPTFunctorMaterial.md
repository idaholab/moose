# RhoFromPTFunctorMaterial

!syntax description /FunctorMaterials/RhoFromPTFunctorMaterial

## Overview

This object takes a fluid properties object and coupled pressure and temperature
variables and provides a functor material property `rho` that is evaluated with
the local pressure and temperature as `rho_from_p_T(_pressure(x), _temperature(x))` where `x`
represents a physical location, e.g. on an element or a face.

!syntax parameters /FunctorMaterials/RhoFromPTFunctorMaterial

!syntax inputs /FunctorMaterials/RhoFromPTFunctorMaterial

!syntax children /FunctorMaterials/RhoFromPTFunctorMaterial
