# Summary of constraint enforcement techniques

## interpolated-ncp-lm-nodal-enforcement.i

- Second order primal, first order LM
- Constraint applied through NodalKernel
- Resultant constraint source applied through Kernel
- No PSPG-type stabilization for LM variable
- Not solveable with `-pc_type hypre -pc_hypre_type boomeramg`
- Local oscillations in LM near discontinuity front and right boundary

Postprocessor Values:
+----------------+----------------+----------------+
| time           | active_lm      | violations     |
+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   2.000000e+00 |   7.000000e+00 |   4.000000e+00 |
|   3.000000e+00 |   1.700000e+01 |   1.100000e+01 |
|   4.000000e+00 |   2.600000e+01 |   1.600000e+01 |
|   5.000000e+00 |   3.400000e+01 |   1.900000e+01 |
|   6.000000e+00 |   4.100000e+01 |   2.400000e+01 |
|   7.000000e+00 |   4.500000e+01 |   1.600000e+01 |
|   8.000000e+00 |   4.900000e+01 |   1.600000e+01 |
|   9.000000e+00 |   5.100000e+01 |   1.600000e+01 |
|   1.000000e+01 |   5.200000e+01 |   1.700000e+01 |
+----------------+----------------+----------------+

## interpolated-ncp-lm-nodal-enforcement-nodal-forces.i

- First order primal, first order LM
- Constraint applied through NodalKernel
- Resultant constraint source applied through NodalKernel
- No PSPG-type stabilization for LM variable
- Not solveable with `-pc_type hypre -pc_hypre_type boomeramg`
- No oscillations

+----------------+----------------+----------------+
| time           | active_lm      | violations     |
+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   2.000000e+00 |   8.000000e+00 |   0.000000e+00 |
|   3.000000e+00 |   1.700000e+01 |   0.000000e+00 |
|   4.000000e+00 |   2.600000e+01 |   0.000000e+00 |
|   5.000000e+00 |   3.400000e+01 |   0.000000e+00 |
|   6.000000e+00 |   4.100000e+01 |   0.000000e+00 |
|   7.000000e+00 |   4.600000e+01 |   0.000000e+00 |
|   8.000000e+00 |   4.900000e+01 |   0.000000e+00 |
|   9.000000e+00 |   5.100000e+01 |   0.000000e+00 |
|   1.000000e+01 |   5.300000e+01 |   0.000000e+00 |
+----------------+----------------+----------------+

## diagonal-ncp-lm-nodal-enforcement.i

- Second order primal, first order LM
- Constraint applied through NodalKernel
- Resultant constraint source applied through Kernel
- Includes PSPG-type stabilization for LM variable
- Solveable with `-pc_type hypre -pc_hypre_type boomeramg`
- Local oscillations in LM near discontinuity front

+----------------+----------------+----------------+
| time           | active_lm      | violations     |
+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e+00 |   3.000000e+00 |   0.000000e+00 |
|   2.000000e+00 |   1.200000e+01 |   9.000000e+00 |
|   3.000000e+00 |   2.100000e+01 |   1.800000e+01 |
|   4.000000e+00 |   3.100000e+01 |   3.000000e+01 |
|   5.000000e+00 |   3.900000e+01 |   3.700000e+01 |
|   6.000000e+00 |   4.600000e+01 |   3.400000e+01 |
|   7.000000e+00 |   5.100000e+01 |   3.200000e+01 |
|   8.000000e+00 |   5.400000e+01 |   3.500000e+01 |
|   9.000000e+00 |   5.600000e+01 |   3.100000e+01 |
|   1.000000e+01 |   5.800000e+01 |   3.400000e+01 |
+----------------+----------------+----------------+

## diagonal-ncp-lm-nodal-enforcement-nodal-forces.i I

- Second order primal, second order LM
- Constraint applied through NodalKernel
- Resultant constraint source applied through NodalKernel
- PSPG-type stabilization for LM variable
- Solveable with `-pc_type hypre -pc_hypre_type boomeramg`
- Global oscillations in LM

+----------------+----------------+----------------+
| time           | active_lm      | violations     |
+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e+00 |   2.000000e+00 |   0.000000e+00 |
|   2.000000e+00 |   1.900000e+01 |   4.000000e+00 |
|   3.000000e+00 |   3.700000e+01 |   6.000000e+00 |
|   4.000000e+00 |   5.600000e+01 |   5.000000e+00 |
|   5.000000e+00 |   7.300000e+01 |   6.000000e+00 |
|   6.000000e+00 |   8.600000e+01 |   6.000000e+00 |
|   7.000000e+00 |   9.600000e+01 |   6.000000e+00 |
|   8.000000e+00 |   1.020000e+02 |   5.000000e+00 |
|   9.000000e+00 |   1.060000e+02 |   6.000000e+00 |
|   1.000000e+01 |   1.090000e+02 |   6.000000e+00 |
+----------------+----------------+----------------+

## diagonal-ncp-lm-nodal-enforcement-nodal-forces.i II

- `cli_args=Outputs/file_base=diagonal-ncp-lm-nodal-enforcement-nodal-forces-mixed
  Variables/lm/order=FIRST -snes_mf_operator -ksp_gmres_restart 100 -ksp_max_it 100'
- Second order primal, first order LM
- Constraint applied through NodalKernel
- Resultant constraint source applied through NodalKernel
- PSPG-type stabilization for LM variable
- Requires `-snes_mf_operator` in order to get the `CoupledForceNodalKernel`
  Jacobian  right for internal node
- Solveable with `-pc_type hypre -pc_hypre_type boomeramg`
- Local oscillations in LM near right boundary

+----------------+----------------+----------------+
| time           | active_lm      | violations     |
+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e+00 |   9.000000e+00 |   0.000000e+00 |
|   2.000000e+00 |   2.200000e+01 |   3.000000e+00 |
|   3.000000e+00 |   3.100000e+01 |   3.000000e+00 |
|   4.000000e+00 |   4.000000e+01 |   3.000000e+00 |
|   5.000000e+00 |   4.800000e+01 |   3.000000e+00 |
|   6.000000e+00 |   5.500000e+01 |   3.000000e+00 |
|   7.000000e+00 |   6.000000e+01 |   3.000000e+00 |
|   8.000000e+00 |   6.400000e+01 |   3.000000e+00 |
|   9.000000e+00 |   6.700000e+01 |   3.000000e+00 |
|   1.000000e+01 |   6.900000e+01 |   3.000000e+00 |
+----------------+----------------+----------------+
