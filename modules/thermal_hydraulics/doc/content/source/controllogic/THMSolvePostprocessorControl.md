# THMSolvePostprocessorControl

This component controls the solve based on a postprocessor value.
If the postprocessor indicates zero, no solve is being done.
Otherwise, the solve is performed.

## Example input syntax

In this example, the problem is only solved between time 0.4 and 0.6s, when the value of
the postprocessor in non-zero.

!listing test/tests/controls/thm_solve_postprocessor_control/test.i block=Functions Postprocessors ControlLogic

!syntax parameters /ControlLogic/THMSolvePostprocessorControl

!syntax inputs /ControlLogic/THMSolvePostprocessorControl

!syntax children /ControlLogic/THMSolvePostprocessorControl
