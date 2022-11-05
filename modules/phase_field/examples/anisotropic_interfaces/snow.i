[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 14
  ny = 14
  xmax = 9
  ymax = 9
  uniform_refine = 3
[]

[Variables]
  [./w]
  [../]
  [./T]
  [../]
[]

[ICs]
  [./wIC]
    type = SmoothCircleIC
    variable = w
    int_width = 0.1
    x1 = 4.5
    y1 = 4.5
    radius = 0.07
    outvalue = 0
    invalue = 1
  [../]
[]

[Kernels]
  [./w_dot]
    type = TimeDerivative
    variable = w
  [../]
  [./anisoACinterface1]
    type = ACInterfaceKobayashi1
    variable = w
    mob_name = M
  [../]
  [./anisoACinterface2]
    type = ACInterfaceKobayashi2
    variable = w
    mob_name = M
  [../]
  [./AllenCahn]
    type = AllenCahn
    variable = w
    mob_name = M
    f_name = fbulk
    coupled_variables = T
  [../]
  [./T_dot]
    type = TimeDerivative
    variable = T
  [../]
  [./CoefDiffusion]
    type = Diffusion
    variable = T
  [../]
  [./w_dot_T]
    type = CoefCoupledTimeDerivative
    variable = T
    v = w
    coef = -1.8
  [../]
[]

[Materials]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = fbulk
    coupled_variables = 'w T'
    constant_names = pi
    constant_expressions = 4*atan(1)
    expression = 'm:=0.9 * atan(10 * (1 - T)) / pi; 1/4*w^4 - (1/2 - m/3) * w^3 + (1/4 - m/2) * w^2'
    derivative_order = 2
    outputs = exodus
  [../]
  [./material]
    type = InterfaceOrientationMaterial
    op = w
  [../]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'M'
    prop_values = '3333.333'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      31'

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-08
  l_max_its = 30

  end_time = 1

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 6
    iteration_window = 2
    dt = 0.0005
    growth_factor = 1.1
    cutback_factor = 0.75
  [../]
  [./Adaptivity]
    initial_adaptivity = 3 # Number of times mesh is adapted to initial condition
    refine_fraction = 0.7 # Fraction of high error that will be refined
    coarsen_fraction = 0.1 # Fraction of low error that will coarsened
    max_h_level = 5 # Max number of refinements used, starting from initial mesh (before uniform refinement)
    weight_names = 'w T'
    weight_values = '1 0.5'
  [../]
[]

[Outputs]
  interval = 5
  exodus = true
[]
