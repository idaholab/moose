[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = .05
    ymax = .05
    nx = 20
    ny = 20
    elem_type = QUAD9
  []
  [./bottom_left]
    type = ExtraNodesetGenerator
    new_boundary = corner
    coord = '0 0'
    input = gen
  [../]
[]


[Preconditioning]
  [./Newton_SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady

  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'bjacobi  lu           NONZERO                   200'
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]

[Variables]
  [velocity]
    family = LAGRANGE_VEC
  []
  [p][]
  [temp]
    initial_condition = 340
    scaling = 1e-4
  []
[]

[ICs]
  [velocity]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
    variable = velocity
  []
[]

[BCs]
  [./velocity_dirichlet]
    type = VectorDirichletBC
    boundary = 'left right bottom top'
    variable = velocity
    # The third entry is to satisfy RealVectorValue
    values = '0 0 0'
  [../]
  # Even though we are integrating by parts, because there are no integrated
  # boundary conditions on the velocity p doesn't appear in the system of
  # equations. Thus we must pin the pressure somewhere in order to ensure a
  # unique solution
  [./p_zero]
    type = DirichletBC
    boundary = corner
    variable = p
    value = 0
  [../]
  [./cold]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 300
  [../]
  [./hot]
    type = DirichletBC
    variable = temp
    boundary = right
    value = 400
  [../]
[]


[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
  []

  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  [../]
  [momentum_advection]
    type = INSADMomentumAdvection
    variable = velocity
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
    integrate_p_by_parts = true
  []
  [./buoyancy]
    type = INSADBoussinesqBodyForce
    variable = velocity
    temperature = temp
    gravity = '0 -9.81 0'
  [../]
  [./gravity]
    type = INSADGravityForce
    variable = velocity
    gravity = '0 -9.81 0'
  [../]
  [supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  []

  [temp_advection]
    type = INSADEnergyAdvection
    variable = temp
  []
  [temp_conduction]
    type = ADHeatConduction
    variable = temp
    thermal_conductivity = 'k'
  [../]
  [temp_supg]
    type = INSADEnergySUPG
    variable = temp
    velocity = velocity
  []
[]

[Materials]
  [./ad_const]
    type = ADGenericConstantMaterial
    # alpha = coefficient of thermal expansion where rho  = rho0 -alpha * rho0 * delta T
    prop_names =  'mu        rho   alpha   k        cp'
    prop_values = '30.74e-6  .5757 2.9e-3  46.38e-3 1054'
  [../]
  [./const]
    type = GenericConstantMaterial
    prop_names =  'temp_ref'
    prop_values = '900'
  [../]
  [ins_mat]
    type = INSADStabilized3Eqn
    velocity = velocity
    pressure = p
    temperature = temp
  []
[]
