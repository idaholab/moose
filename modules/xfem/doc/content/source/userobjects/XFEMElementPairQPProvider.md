# XFEMElementPairQPProvider

!syntax description /UserObjects/XFEMElementPairQPProvider

## Overview

The `XFEMElementPairQPProvider` is a user object that manages stateful
properties along XFEM fragment cuts. It is used by the
[XFEMElementPairMaterialManager](XFEMElementPairMaterialManager.md).

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition_2d.i block=UserObjects/velocity

!syntax parameters /UserObjects/XFEMElementPairQPProvider

!syntax inputs /UserObjects/XFEMElementPairQPProvider

!syntax children /UserObjects/XFEMElementPairQPProvider
