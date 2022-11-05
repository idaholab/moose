[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  elem_type = QUAD
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./c]
  [../]
[]

[ICs]
  [./initial]
    value = 1.0
    variable = u
    type = ConstantIC
  [../]
  [./c_IC]
    int_width = 0.1
    x1 = 0.5
    y1 = 0.5
    radius = 0.25
    outvalue = 0
    variable = c
    invalue = 1
    type = SmoothCircleIC
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
  [./source]
    type = MaskedBodyForce
    variable = u
    value = 1
    mask = mask
  [../]
[]

[Materials]
  [./mask]
    type = ParsedMaterial
    expression = if(c>0.5,0,1)
    property_name = mask
    coupled_variables = c
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
