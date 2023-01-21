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
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  [../]
[]

[AuxVariables]
  [u]
    initial_condition = 1
  []
[]

[Variables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
  [./p]
  [../]
  [temperature][]
[]

[ICs]
  [velocity]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
    variable = velocity
  []
[]

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]
  [./mass_pspg]
    type = INSADMassPSPG
    variable = p
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

  [./momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  [../]

 [./temperature_advection]
   type = INSADEnergyAdvection
   variable = temperature
 [../]

  [./temperature_conduction]
    type = ADHeatConduction
    variable = temperature
    thermal_conductivity = 'k'
  [../]

  [temperature_source]
    type = INSADEnergySource
    variable = temperature
    source_variable = u
  []

  [temperature_supg]
    type = INSADEnergySUPG
    variable = temperature
    velocity = velocity
  []
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

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0
  [../]

  [./temperature_hot]
    type = DirichletBC
    variable = temperature
    boundary = 'bottom'
    value = 1
  [../]

  [./temperature_cold]
    type = DirichletBC
    variable = temperature
    boundary = 'top'
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
    type = INSADStabilized3Eqn
    velocity = velocity
    pressure = p
    temperature = temperature
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

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_levels -ksp_gmres_restart'
  petsc_options_value = 'asm      6                     200'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_max_its = 6
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'u'
  []
[]
