[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '0.5'
    ix = '30 30'
    iy = '20'
    subdomain_id = '1 2'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'interface'
  []
  [break]
    type = BreakBoundaryOnSubdomainGenerator
    input = interface
    boundaries = 'top bottom'
  []
  # second_order = true
[]

[Variables]
  [velocity1]
    family = LAGRANGE_VEC
    # order = SECOND
    block = 1
  []
  [pressure1]
    block = 1
  []
  [velocity2]
    family = LAGRANGE_VEC
    # order = SECOND
    block = 2
  []
  [pressure2]
    block = 2
  []
[]

[AuxVariables]
  [porosity1]
    initial_condition = 1
    block = 1
  []
  [porosity2]
    initial_condition = 0.5
    block = 2
  []
[]

[ICs]
  [vel1]
    type = VectorConstantIC
    variable = velocity1
    x_value = 1
    y_value = 1e-6
    block = 1
  []
  [vel2]
    type = VectorConstantIC
    variable = velocity2
    x_value = 1
    y_value = 1e-6
    block = 2
  []
[]

[Kernels]
  [mass1]
    type = INSADMass
    variable = pressure1
    block = 1
  []
  [pspg1]
    type = INSADMassPSPG
    variable = pressure1
    block = 1
  []
  [momentum_convection1]
    type = INSADMomentumAdvection
    variable = velocity1
    block = 1
    rho = rho
    porosity = porosity1
  []
  [momentum_viscous1]
    type = INSADMomentumViscous
    variable = velocity1
    porosity = porosity1
    block = 1
  []
  [momentum_pressure1]
    type = INSADMomentumPressure
    variable = velocity1
    pressure = pressure1
    integrate_p_by_parts = true
    block = 1
  []
  [momentum_supg1]
    type = INSADMomentumSUPG
    variable = velocity1
    velocity = velocity1
    block = 1
  []

  [mass2]
    type = INSADMass
    variable = pressure2
    block = 2
  []
  [pspg2]
    type = INSADMassPSPG
    variable = pressure2
    block = 2
  []
  [momentum_convection2]
    type = INSADMomentumAdvection
    variable = velocity2
    block = 2
    rho = rho
    porosity = porosity2
  []
  [momentum_viscous2]
    type = INSADMomentumViscous
    variable = velocity2
    porosity = porosity2
    block = 2
  []
  [momentum_pressure2]
    type = INSADMomentumPressure
    variable = velocity2
    pressure = pressure2
    integrate_p_by_parts = true
    block = 2
  []
  [momentum_supg2]
    type = INSADMomentumSUPG
    variable = velocity2
    velocity = velocity2
    block = 2
  []
[]

[InterfaceKernels]
  [interface]
    boundary = interface
    variable = velocity1
    neighbor_var = velocity2
    penalty = 1e6
    type = VectorPenaltyInterfaceDiffusion
  []
[]

[BCs]
  [inlet]
    type = VectorFunctionDirichletBC
    boundary = 'left'
    variable = velocity1
    function_x = '1'
  []
  [walls1]
    boundary = 'top_to_1 bottom_to_1'
    type = ADVectorFunctionDirichletBC
    set_x_comp = false
    set_z_comp = false
    variable = velocity1
  []
  [walls2]
    boundary = 'top_to_2 bottom_to_2'
    type = ADVectorFunctionDirichletBC
    set_x_comp = false
    set_z_comp = false
    variable = velocity2
  []
  [outflow]
    type = INSADMomentumAdvectionOutflowBC
    rho = rho
    porosity = porosity2
    variable = velocity2
    boundary = 'right'
  []
  # [inlet]
  #   type = VectorPenaltyDirichletBC
  #   boundary = 'left'
  #   variable = velocity
  #   x_exact_sln = '1'
  #   penalty = 1e6
  # []
  # [walls]
  #   boundary = 'top bottom'
  #   type = ADVectorPenaltyDirichletBC
  #   penalize_x = false
  #   penalize_z = false
  #   variable = velocity
  #   penalty = 1e6
  # []
[]

[Materials]
  [constant_rho_mu]
    type = ADGenericConstantMaterial
    prop_names =  'rho mu'
    prop_values = '1.1 1e-6'
  []
  [ins1]
    type = INSADTauMaterial
    pressure = pressure1
    velocity = velocity1
    porosity = porosity1
    block = 1
  []
  [ins2]
    type = INSADTauMaterial
    pressure = pressure2
    velocity = velocity2
    porosity = porosity2
    block = 2
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
