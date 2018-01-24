[Mesh]
  type = FileMesh
  file = mug.e
[]

[MeshModifiers]
  [./subdomains]
    type = SubdomainBoundingBox
    top_right = '3 3 3'
    bottom_left = '0 -3 -2.1'
    block_id = '76'
  [../]
[]

[Variables]
  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./aux_elem]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff_convected]
    type = Diffusion
    variable = convected
  [../]
  [./conv]
    # Couple a variable into the convection kernel using local_name = simulationg_name syntax
    type = Convection
    variable = convected
    velocity = '1 1 1'
  [../]
  [./diff_diffused]
    type = Diffusion
    variable = diffused
  [../]
  [./diff_t]
    type = TimeDerivative
    variable = diffused
  [../]
  [./conv_t]
    type = TimeDerivative
    variable = convected
    block = '76'
  [../]
[]

[BCs]
  [./bottom_convected]
    type = DirichletBC
    variable = convected
    boundary = bottom
    value = 1
  [../]
  [./top_convected]
    type = DirichletBC
    variable = convected
    boundary = top
    value = 0
  [../]
  [./bottom_diffused]
    type = DirichletBC
    variable = diffused
    boundary = bottom
    value = 2
  [../]
  [./top_diffused]
    type = DirichletBC
    variable = diffused
    boundary = top
    value = 0
  [../]
[]

[Postprocessors]
  [./func_pp]
    type = FunctionValuePostprocessor
    function = 2*t
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 20
  solve_type = PJFNK
  dt = 0.1
[]

[Outputs]
  exodus = true
[]

[ICs]
  [./aux_ic]
    variable = aux_elem
    max = 10
    seed = 2
    type = RandomIC
  [../]
[]
