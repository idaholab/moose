# Exercises the optional outputs of MoveNodesByParsedExpressionModifier: original node
# coordinates (ref_coord_*), displacement (mv_disp_*), and the per-element density
# adjustment factor (jratio). With displacement_x = 0.1*x the deformation gradient
# is F = diag(1.1, 1, 1), so det(F) = 1.1 and jratio = V_orig/V_new = 1/1.1.
# The output aux variables are created by the user (below) and written by the
# modifier.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[AuxVariables]
  [ref_coord_x]
  []
  [ref_coord_y]
  []
  [ref_coord_z]
  []
  [mv_disp_x]
  []
  [mv_disp_y]
  []
  [mv_disp_z]
  []
  [jratio]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [move]
    type = MoveNodesByParsedExpressionModifier
    block = 0
    displacement_x = '0.1*x'
    original_coordinate_variables = 'ref_coord_x ref_coord_y ref_coord_z'
    displacement_variables = 'mv_disp_x mv_disp_y mv_disp_z'
    density_factor_variable = 'jratio'
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'FINAL'
  []
[]
