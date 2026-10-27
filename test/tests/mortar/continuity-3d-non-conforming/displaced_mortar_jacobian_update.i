!include normal_projection_distorted_quad.i

[Mesh]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Problem]
  type = MortarMeshUpdateTestProblem
[]

[Constraints]
  [mortar]
    use_displaced_mesh = true
  []
[]

[Executioner]
  automatic_scaling = true
  # Force automatic scaling to assemble a standalone Jacobian.
  resid_vs_jac_scaling_param = 0
[]
