[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 32
  ny = 32
  xmax = 0.7
  ymax = 0.7
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
    x1 = 0.35
    y1 = 0.35
    radius = 0.08
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
    coupled_variables = 'T'
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
    coef = -1.8 #This is -K from kobayashi's paper
  [../]
[]

[Materials]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = fbulk
    coupled_variables = 'w T'
    constant_names = 'alpha gamma T_e pi'
    constant_expressions = '0.9 10 1 4*atan(1)'
    expression = 'm:=alpha/pi * atan(gamma * (T_e - T)); 1/4*w^4 - (1/2 - m/3) * w^3 + (1/4 - m/2) * w^2'
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
  solve_type = PJFNK
  scheme = bdf2
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-08
  l_tol = 1e-4
  l_max_its = 30

  dt = 0.001
  num_steps = 6
[]

[Outputs]
  exodus = true
  perf_graph = true
  execute_on = 'INITIAL FINAL'
[]
