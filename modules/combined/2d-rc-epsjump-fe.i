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
  # second_order = true
[]

[Variables]
  [velocity]
    family = LAGRANGE_VEC
    # order = SECOND
  []
  [pressure][]
[]

[AuxVariables]
  [porosity][]
[]

[UserObjects]
  [npr]
    type = NodalPatchRecoveryMaterialProperty
    property = 'porosity'
    patch_polynomial_order = FIRST
    execute_on = 'initial'
  []
[]

[AuxKernels]
  [npr]
    type = NodalPatchRecoveryAux
    execute_on = 'initial'
    nodal_patch_recovery_uo = 'npr'
    variable = porosity
  []
[]

[ICs]
  [vel]
    type = VectorConstantIC
    variable = velocity
    x_value = 1
    y_value = 1e-6
  []
[]

[Kernels]
  [mass]
    type = INSADMass
    variable = pressure
  []
  [pspg]
    type = INSADMassPSPG
    variable = pressure
  []

  [momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
    porosity = porosity
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = pressure
    integrate_p_by_parts = true
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  []
[]

[BCs]
  [inlet]
    type = VectorFunctionDirichletBC
    boundary = 'left'
    variable = velocity
    function_x = '1'
  []
  [walls]
    boundary = 'top bottom'
    type = ADVectorFunctionDirichletBC
    set_x_comp = false
    set_z_comp = false
    variable = velocity
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
  [eps_1]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'porosity'
    prop_values = '1'
  []
  [eps_2]
    type = GenericConstantMaterial
    block = 2
    prop_names = 'porosity'
    prop_values = '0.5'
  []
  [ins]
    type = INSADTauMaterial
    pressure = pressure
    velocity = velocity
    porosity = porosity
    alpha = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
[]

[Outputs]
  exodus = true
[]
