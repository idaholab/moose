# ResidualEvaluationUserObject

!syntax description /UserObjects/ResidualEvaluationUserObject

## Overview

To obtain the most up-to-date residuals from kernel using tagging system. This object is designed for explicit time integration, if user would like to extract residuals from specific kernel object after the solve (And perform calculations in TIMESTEP_END). 

Several components are needed: (1) extra tag with tag name in "Problem" object; (2) tag name in "extra_vector_tags" option in specific kernel; (3) this object in the "UserObject"; (4) TagVectorAux object in the "AuxKernels" to extract residuals to auxvariables.

!alert warning
Please note the "force_preaux = true", "execute_on = 'TIMESTEP_END" are default options to ensure residual tag is updated BEFORE retrieve its value using "TagVectorAux" at TIMESTEP_END.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/2D_slipweakening/tpv2052D.i block=Problem

!listing modules/tensor_mechanics/test/tests/2D_slipweakening/tpv2052D.i block=UserObjects/recompute_residual_tag

!listing modules/tensor_mechanics/test/tests/2D_slipweakening/tpv2052D.i block=AuxKernels/restore_x

!syntax parameters /UserObjects/ResidualEvaluationUserObject

!syntax inputs /UserObjects/ResidualEvaluationUserObject

!syntax children /UserObjects/ResidualEvaluationUserObject
