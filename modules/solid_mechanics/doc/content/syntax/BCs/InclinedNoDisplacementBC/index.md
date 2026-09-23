# Inclined No Displacement Boundary Condition Action System

!syntax description /BCs/InclinedNoDisplacementBC/InclinedNoDisplacementBCAction

## Description

The InclinedNoDisplacementBCAction Action, given in the input file as simply `InclinedNoDisplacementBC`, is designed to simplify the input file when several variables have the same inclined no displacement boundary condition [Inclined no displacement boundary condition](PenaltyInclinedNoDisplacementBC.md)  applied in the normal component.

This action builds the penalty form of the condition. [InclinedNoDisplacementConstraint.md] enforces the same $\mathbf{u}\cdot \mathbf{normal} = 0$ exactly, with one degree of freedom constraint row per node of the support rather than a penalty residual, so it needs no penalty parameter and adds no stiffness. It takes the support normal from the undisplaced mesh once, so use it for a flat or locally flat support under small deformation and keep the penalty form of this action for large sliding on a curved support.

## Example Input Syntax

!listing modules/solid_mechanics/test/tests/inclined_bc/inclined_bc_action.i block=BCs/InclinedNoDisplacementBC

!syntax parameters /BCs/InclinedNoDisplacementBC/InclinedNoDisplacementBCAction

## Associated Actions

!syntax list /BCs/InclinedNoDisplacementBC objects=True actions=False subsystems=False

!syntax list /BCs/InclinedNoDisplacementBC objects=False actions=False subsystems=True

!syntax list /BCs/InclinedNoDisplacementBC objects=False actions=True subsystems=False
