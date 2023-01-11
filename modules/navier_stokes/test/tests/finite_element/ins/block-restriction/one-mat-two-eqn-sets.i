[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 1
    nx = 16
    ny = 8
    elem_type = QUAD9
  []
  [./corner_node_0]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node_0'
    coord = '0 0 0'
    input = gen
  [../]
  [./corner_node_1]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node_1'
    coord = '1 0 0'
    input = corner_node_0
  [../]
  [./subdomain1]
    input = corner_node_1
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1 0 0'
    top_right = '2 1 0'
    block_id = 1
  [../]
  [./break_boundary]
    input = subdomain1
    type = BreakBoundaryOnSubdomainGenerator
  [../]
  [./interface0]
    type = SideSetsBetweenSubdomainsGenerator
    input = break_boundary
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'interface0'
  [../]
  [./interface1]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface0
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'interface1'
  [../]
[]

[Variables]
  [velocity0]
    order = SECOND
    family = LAGRANGE_VEC
  []
  [T0]
    order = SECOND
    [InitialCondition]
      type = ConstantIC
      value = 1.0
    []
  []
  [p0]
  []
[]

[Kernels]
  [./mass0]
    type = INSADMass
    variable = p0
    block = 0
  [../]
  [./momentum_time0]
    type = INSADMomentumTimeDerivative
    variable = velocity0
    block = 0
  [../]
  [./momentum_convection0]
    type = INSADMomentumAdvection
    variable = velocity0
    block = 0
  [../]
  [./momentum_viscous0]
    type = INSADMomentumViscous
    variable = velocity0
    block = 0
  [../]
  [./momentum_pressure0]
    type = INSADMomentumPressure
    variable = velocity0
    pressure = p0
    integrate_p_by_parts = true
    block = 0
  [../]
  [./temperature_time0]
    type = INSADHeatConductionTimeDerivative
    variable = T0
    block = 0
  [../]
  [./temperature_advection0]
    type = INSADEnergyAdvection
    variable = T0
    block = 0
  [../]
  [./temperature_conduction0]
    type = ADHeatConduction
    variable = T0
    thermal_conductivity = 'k'
    block = 0
  [../]

  [./mass1]
    type = INSADMass
    variable = p0
    block = 1
  [../]
  [./momentum_time1]
    type = INSADMomentumTimeDerivative
    variable = velocity0
    block = 1
  [../]
  [./momentum_convection1]
    type = INSADMomentumAdvection
    variable = velocity0
    block = 1
  [../]
  [./momentum_viscous1]
    type = INSADMomentumViscous
    variable = velocity0
    block = 1
  [../]
  [./momentum_pressure1]
    type = INSADMomentumPressure
    variable = velocity0
    pressure = p0
    integrate_p_by_parts = true
    block = 1
  [../]
  [./temperature_time1]
    type = INSADHeatConductionTimeDerivative
    variable = T0
    block = 1
  [../]
  [./temperature_advection1]
    type = INSADEnergyAdvection
    variable = T0
    block = 1
  [../]
  [./temperature_conduction1]
    type = ADHeatConduction
    variable = T0
    thermal_conductivity = 'k'
    block = 1
  [../]
[]

[BCs]
  [./no_slip0]
    type = VectorFunctionDirichletBC
    variable = velocity0
    boundary = 'bottom_to_0 interface0 left'
  [../]
  [./lid0]
    type = VectorFunctionDirichletBC
    variable = velocity0
    boundary = 'top_to_0'
    function_x = 'lid_function0'
  [../]
  [./T_hot0]
    type = DirichletBC
    variable = T0
    boundary = 'bottom_to_0'
    value = 1
  [../]
  [./T_cold0]
    type = DirichletBC
    variable = T0
    boundary = 'top_to_0'
    value = 0
  [../]
  [./pressure_pin0]
    type = DirichletBC
    variable = p0
    boundary = 'pinned_node_0'
    value = 0
  [../]

  [./no_slip1]
    type = VectorFunctionDirichletBC
    variable = velocity0
    boundary = 'bottom_to_1 interface1 right'
  [../]
  [./lid1]
    type = VectorFunctionDirichletBC
    variable = velocity0
    boundary = 'top_to_1'
    function_x = 'lid_function1'
  [../]
  [./T_hot1]
    type = DirichletBC
    variable = T0
    boundary = 'bottom_to_1'
    value = 1
  [../]
  [./T_cold1]
    type = DirichletBC
    variable = T0
    boundary = 'top_to_1'
    value = 0
  [../]
[]

[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu cp k'
    prop_values = '1  1  1  .01'
  [../]
  [ins_mat0]
    type = INSAD3Eqn
    velocity = velocity0
    pressure = p0
    temperature = T0
    block = '0 1'
  []
[]

[Functions]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
  [./lid_function0]
    type = ParsedFunction
    expression = '4*x*(1-x)'
  [../]
  [./lid_function1]
    type = ParsedFunction
    expression = '4*(x-1)*(2-x)'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Transient
  # Run for 100+ timesteps to reach steady state.
  num_steps = 5
  dt = .5
  dtmin = .5
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_levels -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      2               ilu          4                     NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
[]

[Outputs]
  exodus = true
[]
