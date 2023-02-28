# CartesianMortarMechanicalContact

!syntax description /Constraints/CartesianMortarMechanicalContact

## Overview

It applies mortar generalized forces from Lagrange multipliers defined in the global Cartesian frame of reference.
This can be used for both frictional and frictionless contact.

## Example Input File Syntax

In this example, we solve a cylinder-on-plane plane strain frictional problem with Cartesian Lagrange multipliers.
Subdomain `10000` and `10001` are lower dimensional blocks (1D here) created from the `2` and `3` sidesets respectively,
which are on both sides of the contact axis.

!listing tests/mortar_cartesian_lms/cylinder_friction_cartesian.i block=Constraints

!syntax parameters /Constraints/CartesianMortarMechanicalContact

!syntax inputs /Constraints/CartesianMortarMechanicalContact

!syntax children /Constraints/CartesianMortarMechanicalContact
