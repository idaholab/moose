# ExplicitDynamicsContact

## Description

The `ExplicitDynamicsContact` block can be used to specify parameters related to mechanical contact enforcement in
MOOSE simulations. The [ExplicitDynamicsContact](/actions/ExplicitDynamicsContactAction.md) is associated with this
input block, and is the class that performs the associated model setup tasks. For details on the enforcement of contact
constraints in explicit dynamics simulations, see [ExplicitDynamicsContactConstraint](/constraints/ExplicitDynamicsContactConstraint.md)

Currently, only normal contact is supported with explicit dynamics.

!syntax parameters /ExplicitDynamicsContact/ExplicitDynamicsContactAction

!syntax inputs /ExplicitDynamicsContact/ExplicitDynamicsContactAction

!syntax children /ExplicitDynamicsContact/ExplicitDynamicsContactAction
