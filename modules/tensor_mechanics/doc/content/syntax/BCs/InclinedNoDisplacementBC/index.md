# Inclined No Displacement Boundary Condition Action System

!syntax description /BCs/InclinedNoDisplacementBC/InclinedNoDisplacementBCAction

## Description

The InclinedNoDisplacementBCAction Action, given in the input file as simply `InclinedNoDisplacementBC`, is designed to simplify the input file when several variables have the same inclined no displacement boundary condition [Inclined no displacement boundary condition](PenaltyInclinedNoDisplacementBC.md)  applied in the normal component.

## Example Input Syntax

!listing modules/tensor_mechanics/test/tests/inclined_bc/inclined_bc_action.i block=BCs/InclinedNoDisplacementBC

!syntax parameters /BCs/InclinedNoDisplacementBC/InclinedNoDisplacementBCAction

## Associated Actions

!syntax list /BCs/InclinedNoDisplacementBC objects=True actions=False subsystems=False

!syntax list /BCs/InclinedNoDisplacementBC objects=False actions=False subsystems=True

!syntax list /BCs/InclinedNoDisplacementBC objects=False actions=True subsystems=False
