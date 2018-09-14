# XFEMEqualValueAtInterface

!syntax description /Constraints/XFEMEqualValueAtInterface

## Description

The `XFEMEqualValueAtInterface` enforces $u_{\Gamma^+} = u_{\Gamma^-}= a$ at the interface using a penalty approach.

## Example Input File Syntax

!listing modules/xfem/test/tests/single_var_constraint_2d/equal_value.i block=Constraints/xfem_constraint

!syntax parameters /Constraints/XFEMEqualValueAtInterface

!syntax inputs /Constraints/XFEMEqualValueAtInterface

!syntax children /Constraints/XFEMEqualValueAtInterface

!bibtex bibliography
