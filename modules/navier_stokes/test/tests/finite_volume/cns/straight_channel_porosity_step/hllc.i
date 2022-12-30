p_initial=1.01e5
T=273.15
# u refers to the superficial velocity
u_in=1

[GlobalParams]
  fp = fp
  two_term_boundary_expansion = true
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 18
    nx = 180
  []
  [to_pt5]
    input = cartesian
    type = SubdomainBoundingBoxGenerator
    bottom_left = '2 0 0'
    top_right = '4 1 0'
    block_id = 1
  []
  [pt5]
    input = to_pt5
    type = SubdomainBoundingBoxGenerator
    bottom_left = '4 0 0'
    top_right = '6 1 0'
    block_id = 2
  []
  [to_pt25]
    input = pt5
    type = SubdomainBoundingBoxGenerator
    bottom_left = '6 0 0'
    top_right = '8 1 0'
    block_id = 3
  []
  [pt25]
    input = to_pt25
    type = SubdomainBoundingBoxGenerator
    bottom_left = '8 0 0'
    top_right = '10 1 0'
    block_id = 4
  []
  [to_pt5_again]
    input = pt25
    type = SubdomainBoundingBoxGenerator
    bottom_left = '10 0 0'
    top_right = '12 1 0'
    block_id = 5
  []
  [pt5_again]
    input = to_pt5_again
    type = SubdomainBoundingBoxGenerator
    bottom_left = '12 0 0'
    top_right = '14 1 0'
    block_id = 6
  []
  [to_one]
    input = pt5_again
    type = SubdomainBoundingBoxGenerator
    bottom_left = '14 0 0'
    top_right = '16 1 0'
    block_id = 7
  []
  [one]
    input = to_one
    type = SubdomainBoundingBoxGenerator
    bottom_left = '16 0 0'
    top_right = '18 1 0'
    block_id = 8
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Problem]
  fv_bcs_integrity_check = false
[]

[Variables]
  [pressure]
    type = MooseVariableFVReal
    initial_condition = ${p_initial}
  []
  [sup_vel_x]
    type = MooseVariableFVReal
    initial_condition = 1
    scaling = 1e-2
  []
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = ${T}
    scaling = 1e-5
  []
[]

[AuxVariables]
  [vel_x]
    type = MooseVariableFVReal
  []
  [sup_mom_x]
    type = MooseVariableFVReal
  []
  [rho]
    type = MooseVariableFVReal
  []
  [worst_courant]
    type = MooseVariableFVReal
  []
  [porosity]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [vel_x]
    type = ADMaterialRealAux
    variable = vel_x
    property = vel_x
    execute_on = 'timestep_end'
  []
  [sup_mom_x]
    type = ADMaterialRealAux
    variable = sup_mom_x
    property = superficial_rhou
    execute_on = 'timestep_end'
  []
  [rho]
    type = ADMaterialRealAux
    variable = rho
    property = rho
    execute_on = 'timestep_end'
  []
  [worst_courant]
    type = Courant
    variable = worst_courant
    u = sup_vel_x
    execute_on = 'timestep_end'
  []
  [porosity]
    type = MaterialRealAux
    variable = porosity
    property = porosity
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_advection]
    type = PCNSFVMassHLLC
    variable = pressure
  []

  [momentum_advection]
    type = PCNSFVMomentumHLLC
    variable = sup_vel_x
    momentum_component = 'x'
  []
  [eps_grad]
    type = PNSFVPGradEpsilon
    variable = sup_vel_x
    momentum_component = 'x'
    epsilon_function = 'eps'
  []

  [energy_advection]
    type = PCNSFVFluidEnergyHLLC
    variable = T_fluid
  []
[]

[FVBCs]
  [rho_left]
    type = PCNSFVStrongBC
    boundary = 'left'
    variable = pressure
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'mass'
  []
  [rhou_left]
    type = PCNSFVStrongBC
    boundary = 'left'
    variable = sup_vel_x
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_left]
    type = PCNSFVStrongBC
    boundary = 'left'
    variable = T_fluid
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'energy'
  []
  [rho_right]
    type = PCNSFVStrongBC
    boundary = 'right'
    variable = pressure
    pressure = ${p_initial}
    eqn = 'mass'
  []
  [rhou_right]
    type = PCNSFVStrongBC
    boundary = 'right'
    variable = sup_vel_x
    pressure = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_right]
    type = PCNSFVStrongBC
    boundary = 'right'
    variable = T_fluid
    pressure = ${p_initial}
    eqn = 'energy'
  []

  # Use these to help create more accurate cell centered gradients for cells adjacent to boundaries
  [T_left]
    type = FVDirichletBC
    variable = T_fluid
    value = ${T}
    boundary = 'left'
  []
  [sup_vel_left]
    type = FVDirichletBC
    variable = sup_vel_x
    value = ${u_in}
    boundary = 'left'
  []
  [p_right]
    type = FVDirichletBC
    variable = pressure
    value = ${p_initial}
    boundary = 'right'
  []
[]

[Functions]
  [ud_in]
    type = ParsedVectorFunction
    expression_x = '${u_in}'
  []
  [eps]
    type = ParsedFunction
    expression = 'if(x < 2, 1,
             if(x < 4, 1 - .5 / 2 * (x - 2),
             if(x < 6, .5,
             if(x < 8, .5 - .25 / 2 * (x - 6),
             if(x < 10, .25,
             if(x < 12, .25 + .25 / 2 * (x - 10),
             if(x < 14, .5,
             if(x < 16, .5 + .5 / 2 * (x - 14),
                1))))))))'
  []
[]

[Materials]
  [var_mat]
    type = PorousPrimitiveVarMaterial
    pressure = pressure
    T_fluid = T_fluid
    superficial_vel_x = sup_vel_x
    porosity = porosity
  []
  [porosity]
    type = GenericFunctionMaterial
    prop_names = 'porosity'
    prop_values = 'eps'
  []
[]

[Executioner]
  solve_type = NEWTON
  line_search = 'bt'
  type = Steady
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
  checkpoint = true
[]

[Debug]
  show_var_residual_norms = true
[]
