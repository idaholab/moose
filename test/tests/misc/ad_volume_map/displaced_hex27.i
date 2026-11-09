# A HEX27 element has 81 displacement dofs, more than the default AD derivative size. The only AD
# object is a nodal BC, which needs no AD element geometry, so none is built.
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  order = SECOND
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = HEX27
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [diff_x]
    type = Diffusion
    variable = disp_x
    use_displaced_mesh = true
  []
  [diff_y]
    type = Diffusion
    variable = disp_y
    use_displaced_mesh = true
  []
  [diff_z]
    type = Diffusion
    variable = disp_z
    use_displaced_mesh = true
  []
[]

[BCs]
  [left_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [right_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.1
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [back_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
[]

[Postprocessors]
  [average_disp_x]
    type = ElementAverageValue
    variable = disp_x
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
