# PolycrystalVariablesAction

!syntax description /Variables/PolycrystalVariables/PolycrystalVariablesAction

## Overview

This action creates a series of variables with a common naming convention. The required input parameter `op_num` defines how many variables will be generated. Each variable is named based on the `var_name_base` parameter; e.g. for `op_num` = 3, `var_name_base` = `gr`, three nonlinear variables are created (`gr0`, `gr1`, and `gr2`).

This action can be used to create variables used for a phase field grain growth model, but can also be used anytime a series of numbered variables is desired.

+Variables+

- Creates a total of `op_num` variables

  - Variables named `var_name_base`0, `var_name_base`1, $\ldots$, `var_name_base`+`op_num`.

## Example Input File Syntax

The `PolycrystalVariablesAction` is accessed through the `Variables` block, as shown below:

!listing modules/phase_field/test/tests/grain_growth/test.i block=GlobalParams

!listing modules/phase_field/test/tests/grain_growth/test.i block=Variables

!syntax parameters /Variables/PolycrystalVariables/PolycrystalVariablesAction

!syntax children /Variables/PolycrystalVariables/PolycrystalVariablesAction
