[Mesh]
  file = circle-tris.e
[]

[Functions]
  [./all_bc_fn]
    type = ParsedFunction
    value = x*x+y*y
  [../]
  
  [./f_fn]
    type = ParsedFunction
    value = -4
  [../]
  
  [./analytical_normal_x]
    type = ParsedFunction
    value = x
  [../]
  [./analytical_normal_y]
    type = ParsedFunction
    value = y
  [../]
[]

[NodalNormals]
[]

[AuxVariables]
  [./normal_x]
    family = LAGRANGE
    order = FIRST
  [../]
  [./normal_y]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[AuxKernels]
  [./nx]
    type = NodalNormalComponentAux
    variable = normal_x
    component = 0
  [../]
  [./ny]
    type = NodalNormalComponentAux
    variable = normal_y
    component = 1
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = UserForcingFunction
    variable = u
    function = f_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '1'
    function = 'all_bc_fn'
  [../]
[]

[Postprocessors]
  [./nx_pps]
    type = NodalL2Error
    variable = normal_x
    boundary = '1'
    function = analytical_normal_x
  [../]
  [./ny_pps]
    type = NodalL2Error
    variable = normal_y
    boundary = '1'
    function = analytical_normal_y
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
  nl_rel_tol = 1e-13
[]

[Output]
  exodus = true
[]