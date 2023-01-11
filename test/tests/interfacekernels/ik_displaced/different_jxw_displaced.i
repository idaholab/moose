[Mesh]
  displacements = 'disp_x disp_y'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    xmax = 2
    ny = 1
    ymax = 1
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./break_boundary]
    input = subdomain1
    type = BreakMeshByBlockGenerator
  [../]
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
    use_displaced_mesh = true
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = InterfacialSource
    variable = u
    neighbor_var = u
    boundary = interface
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./u]
    type = DirichletBC
    variable = u
    boundary = 'left right'
    value = 0
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]

[Functions]
  [./disp_x_func]
    type = ParsedFunction
    expression = 1
  [../]
  [./disp_y_func]
    type = ParsedFunction
    expression = y
  [../]
[]

[ICs]
  [./disp_x_ic]
    block = 0
    function = disp_x_func
    variable = disp_x
    type = FunctionIC
  [../]
  [./disp_y_ic]
    block = 0
    function = disp_y_func
    variable = disp_y
    type = FunctionIC
  [../]
[]
