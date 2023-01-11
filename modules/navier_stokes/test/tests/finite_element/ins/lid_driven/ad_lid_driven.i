[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = 16
    ny = 16
    elem_type = QUAD9
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  [../]
[]

[AuxVariables]
  [vel_x]
    order = SECOND
  []
  [vel_y]
    order = SECOND
  []
[]

[AuxKernels]
  [vel_x]
    type = VectorVariableComponentAux
    variable = vel_x
    vector_variable = velocity
    component = 'x'
  []
  [vel_y]
    type = VectorVariableComponentAux
    variable = vel_y
    vector_variable = velocity
    component = 'y'
  []
[]

[Variables]
  [./velocity]
    order = SECOND
    family = LAGRANGE_VEC
  [../]

  [./T]
    order = SECOND
    [./InitialCondition]
      type = ConstantIC
      value = 1.0
    [../]
  [../]

  [./p]
  [../]
[]

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]

  [./momentum_time]
    type = INSADMomentumTimeDerivative
    variable = velocity
  [../]

  [./momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  [../]

  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  [../]

  [./momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
    integrate_p_by_parts = true
  [../]

 [./temperature_time]
   type = INSADHeatConductionTimeDerivative
   variable = T
 [../]

 [./temperature_advection]
   type = INSADEnergyAdvection
   variable = T
 [../]

 [./temperature_conduction]
   type = ADHeatConduction
   variable = T
   thermal_conductivity = 'k'
 [../]
[]

[BCs]
  [./no_slip]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'bottom right left'
  [../]

  [./lid]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'top'
    function_x = 'lid_function'
  [../]

  [./T_hot]
    type = DirichletBC
    variable = T
    boundary = 'bottom'
    value = 1
  [../]

  [./T_cold]
    type = DirichletBC
    variable = T
    boundary = 'top'
    value = 0
  [../]

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0
  [../]
[]

[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu cp k'
    prop_values = '1  1  1  .01'
  [../]
  [ins_mat]
    type = INSAD3Eqn
    velocity = velocity
    pressure = p
    temperature = T
  []
[]

[Functions]
  [./lid_function]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
    type = ParsedFunction
    expression = '4*x*(1-x)'
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
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = 'asm      2               ilu          4'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
[]

[Outputs]
  file_base = lid_driven_out
  exodus = true
  perf_graph = true
[]
