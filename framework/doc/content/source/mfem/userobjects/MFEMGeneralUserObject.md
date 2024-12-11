# MFEMGeneralUserObject

!syntax description /UserObjects/MFEMGeneralUserObject

## Overview

`MFEMGeneralUserObject` is a base class for general MFEM user objects, derived from
`GeneralUserObject`. `MFEMGeneralUserObject` adds the `getMFEMProblem()` method for fetching the a
reference to the [`MFEMProblem`](problem/MFEMProblem.md) directly, to minimise the number of casts
required.

!syntax parameters /UserObjects/MFEMGeneralUserObject

!syntax inputs /UserObjects/MFEMGeneralUserObject

!syntax children /UserObjects/MFEMGeneralUserObject
