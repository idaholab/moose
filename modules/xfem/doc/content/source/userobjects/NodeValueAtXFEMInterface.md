# NodeValueAtXFEMInterface

!syntax description /UserObjects/NodeValueAtXFEMInterface

## Overview

The values of field variables on either side of a moving interface are often needed to define the interface velocity. These variables and their gradients might be discontinuous across the interface. The fact that the interface does not lie on the standard quadrature points makes it difficult to get the quantities on the interface using existing functions in MOOSE.

`NodeValueAtXFEMInterface` provides functions to return the value of a field variable and its gradient on the positive and negative sides of the interface. The `NodeValueAtXFEMInterface` object is used by the `XFEMMovingInterfaceVelocityBase` derived class to calculate the velocity of the interface for moving interface problems.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition_2d.i block=UserObjects/value_uo

!syntax parameters /UserObjects/NodeValueAtXFEMInterface

!syntax inputs /UserObjects/NodeValueAtXFEMInterface

!syntax children /UserObjects/NodeValueAtXFEMInterface
