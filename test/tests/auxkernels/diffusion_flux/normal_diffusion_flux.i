[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 1.0
  ymax = 1.0
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./T]
  [../]
  [./flux_n]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./T]
    type = ParsedFunction
    expression = 'x*x*y*y+1'
  [../]
[]

[ICs]
  [./T]
    type = FunctionIC
    variable = T
    function = T
  [../]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = dummy
  [../]
[]

[AuxKernels]
  [./flux_n]
    type = DiffusionFluxAux
    diffusivity = 'thermal_conductivity'
    variable = flux_n
    diffusion_variable = T
    component = normal
    boundary = 'left right'
    check_boundary_restricted = false
  [../]
[]

[Materials]
  [./k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '10'
  [../]
[]

[Postprocessors]
  [flux_right]
    type = SideIntegralVariablePostprocessor
    variable = flux_n
    boundary = 'right'
  []
  [flux_right_exact]
    type = SideFluxIntegral
    variable = T
    diffusivity = 'thermal_conductivity'
    boundary = 'right'
  []
  [flux_left]
    type = SideIntegralVariablePostprocessor
    variable = flux_n
    boundary = 'left'
  []
  [flux_left_exact]
    type = SideFluxIntegral
    variable = T
    diffusivity = 'thermal_conductivity'
    boundary = 'left'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  hide = 'dummy'
[]
