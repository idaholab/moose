# XFEMPhaseTransitionMovingInterfaceVelocity

!syntax description /UserObjects/XFEMPhaseTransitionMovingInterfaceVelocity

## Overview

The `XFEMPhaseTransitionMovingInterfaceVelocity` calculates an interface velocity that is given as $v = \frac{[[ {D}\nabla {u}\cdot\text{n}]]}{[[ {u}]] + u_0}$.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition_2d.i block=UserObjects/velocity

!syntax parameters /UserObjects/XFEMPhaseTransitionMovingInterfaceVelocity

!syntax inputs /UserObjects/XFEMPhaseTransitionMovingInterfaceVelocity

!syntax children /UserObjects/XFEMPhaseTransitionMovingInterfaceVelocity
