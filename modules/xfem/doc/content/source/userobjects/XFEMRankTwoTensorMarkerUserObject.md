# XFEMRankTwoTensorMarkerUserObject

!syntax description /UserObjects/XFEMRankTwoTensorMarkerUserObject

## Overview

This object is used to mark elements to be cut by XFEM based on a scalar extracted from a specified RankTwoTensor (used to store stresses and strains in TensorMechanics), such as a principal stress or a component of stress. All of the standard scalar types that can be extracted from RankTwoTensors are available. If the scalar exceeds a user-specified threshold, the crack extends into the current element. 

The threshold is provided as a coupled variable. This can be specified as either a constant value or as the name of that variable. This is useful for introducing randomness in the strength, by using an AuxVariable that has been initialized with a random initial condition.

The determination of whether an element cracks is based either on the maximum value of the scalar quantity at all of the quadrature points in an element, or on the average value. Cracks are allowed to propagate into new elements that exceed the criterion, or to initiate on specified boundaries.

## Example Input File Syntax

!listing test/tests/solid_mechanics_basic/crack_propagation_2d.i block=UserObjects/xfem_marker_uo

!syntax parameters /UserObjects/XFEMRankTwoTensorMarkerUserObject

!syntax inputs /UserObjects/XFEMRankTwoTensorMarkerUserObject

!syntax children /UserObjects/XFEMRankTwoTensorMarkerUserObject
