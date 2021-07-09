[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
  [break]
    type = BreakMeshByBlockGenerator
    input = interface
  []
  displacements = 'disp_x'
[]

[AuxVariables]
  [disp_x][]
[]

[ICs]
  [right]
    type = ConstantIC
    variable = disp_x
    block = 1
    value = 1
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    block = '0'
  []

  [v]
    order = FIRST
    family = LAGRANGE
    block = '1'
  []
[]

[Kernels]
  [diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 0
  []
  [diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
  []
[]

[InterfaceKernels]
  [penalty_interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [right]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [block0]
    type = GenericConstantMaterial
    block = '0'
    prop_names = 'D'
    prop_values = '4'
  []
  [block1]
    type = GenericConstantMaterial
    block = '1'
    prop_names = 'D'
    prop_values = '2'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [area]
    type = AreaPostprocessor
    use_displaced_mesh = true
    boundary = 'right'
  []
[]
