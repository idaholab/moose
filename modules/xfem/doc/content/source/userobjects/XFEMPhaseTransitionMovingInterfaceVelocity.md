# XFEMPhaseTransitionMovingInterfaceVelocity

!syntax description /UserObjects/XFEMPhaseTransitionMovingInterfaceVelocity

## Description

The `XFEMPhaseTransitionMovingInterfaceVelocity` calculates an interface velocity that is given as $v = \frac{[[ {D}\nabla {u}\cdot\text{n}]]}{[[ {u}]] + u_0}$. The current implementation only supports the case in which the interface is moving horizontally.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition.i block=UserObjects/velocity

!syntax parameters /UserObjects/XFEMPhaseTransitionMovingInterfaceVelocity

!syntax inputs /UserObjects/XFEMPhaseTransitionMovingInterfaceVelocity

!syntax children /UserObjects/XFEMPhaseTransitionMovingInterfaceVelocity

!bibtex bibliography
