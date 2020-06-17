[Mesh]
  file = nodal_normals_test_offset_nonmatching_gap.e
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [disp_x][]
  [disp_y][]
[]

[Materials]
  [gap]
    type = QuadratureLocatorTestMaterial
    boundary = 1
    paired_boundary = 2
  []
[]

[Variables]
  [T][]
[]

[Kernels]
  [T]
    type = Diffusion
    variable = T
  []
[]

[Constraints]
  [demo]
    type = TiedValueConstraint
    variable = T
    primary_variable = T
    secondary = 1
    primary = 2
  []
[]

[Executioner]
  type = Steady
[]
