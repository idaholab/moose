# CNSFVMomImplicitPressureBC

!syntax description /FVBCs/CNSFVMomImplicitPressureBC

## Overview

This object adds a boundary pressure flux based on cell interior (implicit)
information. This is the free-flow analog of
[PCNSFVImplicitMomentumPressureBC.md] although this object can also be used with
porosity by setting `include_porosity = true` in the input file. This object
will then look for a material property named `porosity`.

!syntax parameters /FVBCs/CNSFVMomImplicitPressureBC

!syntax inputs /FVBCs/CNSFVMomImplicitPressureBC

!syntax children /FVBCs/CNSFVMomImplicitPressureBC
