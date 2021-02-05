# XFEMElementPairMaterialManager

!syntax description /UserObjects/XFEMElementPairMaterialManager

## Overview

The `XFEMElementPairMaterialManager` is a user object that manages stateful
properties along XFEM fragmet cuts. It is used by constratints derived from
`XFEMMaterialManagerConstraint`, such as
[XFEMCohesiveConstraint](XFEMCohesiveConstraint.md).

## Example Input File Syntax

!listing modules/xfem/test/tests/solid_mechanics_basic/czm_test.i block=UserObjects/manager

!syntax parameters /UserObjects/XFEMElementPairMaterialManager

!syntax inputs /UserObjects/XFEMElementPairMaterialManager

!syntax children /UserObjects/XFEMElementPairMaterialManager
