## ElementAdaptivityLevelAux

!syntax description /AuxKernels/ElementAdaptivityLevelAux

## Description

ElementAdaptivityLevelAux stores the element hierarchy (how many times the element went through
refinement process during AMR) in an aux variable. If the elements don't go through refinement then
by default its hierarchy is zero. 

## Example Input Syntax

As an example, the syntax below stores the element hierarchy in an aux variable.

!listing test/tests/auxkernels/element_adaptivity_level_aux/element_hierarchy_test.i
block=AuxKernels

!syntax parameters /AuxKernels/ElementAdaptivityLevelAux

!syntax inputs /AuxKernels/ElementAdaptivityLevelAux