# Crack Front Definition

!syntax description /UserObjects/CrackFrontDefinition

## Description

This object is used in the computation of fracture domain integrals. It is used to store information about the location of the crack front, and provides functions used by other objects involved in fracture integral calculation. It is not necessary to define this block in the input file, as it can be set up using the [DomainIntegralAction](/DomainIntegralAction.md).

## Example Input File Syntax

!listing modules/xfem/test/tests/crack_tip_enrichment/edge_crack_2d.i block=UserObjects/crack_tip

!syntax parameters /UserObjects/CrackFrontDefinition

!syntax inputs /UserObjects/CrackFrontDefinition

!syntax children /UserObjects/CrackFrontDefinition
