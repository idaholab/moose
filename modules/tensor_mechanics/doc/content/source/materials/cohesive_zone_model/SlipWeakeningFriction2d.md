# SlipWeakeningFriction2d

!syntax description /Materials/SlipWeakeningFriction2d

## Overview

Material Object for slip weakening friction law in 2D. This material object inherits from Cohesive Zone Model, provides the capability to perform dynamic rupture modeling in explicit time integration.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/2D_slipweakening/tpv2052D.i block=Materials/czm_mat

!syntax parameters /Materials/SlipWeakeningFriction2d
!syntax inputs /Materials/SlipWeakeningFriction2d
!syntax children /Materials/SlipWeakeningFriction2d
