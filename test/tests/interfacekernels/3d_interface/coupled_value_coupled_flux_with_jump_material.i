[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  xmax = 2
  ny = 2
  ymax = 2
  nz = 2
  zmax = 2
[]

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '0 0 0'
    top_right = '1 1 1'
    block_id = 1
  [../]
  [./break_boundary]
    depends_on = subdomain1
    type = BreakBoundaryOnSubdomain
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = break_boundary
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]

# [AuxVariables]
#   [./jump_var]
#     order = CONSTANT
#     family = MONOMIAL
#   [../]
# []

[Kernels]
  [./diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 0
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
  [../]
  [./source_u]
    type = BodyForce
    variable = u
    value = 1
  [../]
[]

# [AuxKernels]
#   [jump_aux]
#     type = MaterialRealAux
#     boundary = master0_interface
#     property = jump
#     variable = jump_var
#   []
# []


[InterfaceKernels]
  [./interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = master0_interface
    penalty = 1e6
    jump_prop_name = jump
  [../]
[]

[Materials]
  [./jump]
    type = JumpInterfaceMaterial
    var = u
    neighbor_var = v
    boundary = master0_interface
  [../]
[]

[BCs]
  [./u]
    type = VacuumBC
    variable = u
    boundary = 'left_to_0 bottom_to_0 back_to_0 right top front'
  [../]
  [./v]
    type = VacuumBC
    variable = v
    boundary = 'left_to_1 bottom_to_1 back_to_1'
  [../]
[]

[Postprocessors]
  [./u_int]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 0
  [../]
  [./v_int]
    type = ElementIntegralVariablePostprocessor
    variable = v
    block = 1
  [../]
  [interface_var_jump]
    type = InterfaceAverageVariableValuePostprocessor
    interface_value_type = jump_abs
    variable = u
    neighbor_variable = v
    execute_on = TIMESTEP_END
    boundary = master0_interface
  []
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
  print_linear_residuals = true
[]
