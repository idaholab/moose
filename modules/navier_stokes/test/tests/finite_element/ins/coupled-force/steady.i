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

[Variables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
  [./p]
  [../]
  [u]
    family = LAGRANGE_VEC
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

  [momentum_coupled_force]
    type = INSADMomentumCoupledForce
    variable = velocity
    coupled_vector_var = u
  []

  [./momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  [../]

  [u_diff]
    type = VectorDiffusion
    variable = u
  []
[]

[BCs]
  [./no_slip]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'bottom right left top'
  [../]

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0
  [../]

  [u_left]
    type = VectorFunctionDirichletBC
    variable = u
    boundary = 'left'
    function_x = 1
    function_y = 1
  []

  [u_right]
    type = VectorFunctionDirichletBC
    variable = u
    boundary = 'right'
    function_x = -1
    function_y = -1
  []
[]

[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
  [ins_mat]
    type = INSADTauMaterial
    velocity = velocity
    pressure = p
  []
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
  exodus = true
[]
