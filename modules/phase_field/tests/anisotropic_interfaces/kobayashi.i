[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 10
  xmax = 1
  ymax = 1
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
    block = 0
    int_width = 0.01
    x1 = 0.5
    y1 = 0.5
    radius = 0.25
    outvalue = 0
    invalue = 1
  [../]
[]

[AuxVariables]
  [./m]
    order = CONSTANT
    family = MONOMIAL
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
    args = m
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

[AuxKernels]
  [./m]
    type = ParsedAux
    variable = m
    args = T
    constant_names = pi
    constant_expressions = 3.14159265359
    function = '0.9*atan(10*(1-T))/pi'
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = fbulk
    args = 'w m'
    function = '1/4*w^4 - (1/2 - m/3)*w^3 + (1/4-m/2)*w^2'
  [../]
  [./material]
    type = InterfaceOrientationMaterial
    block = 0
    c = w
  [../]
  [./consts]
    type = GenericConstantMaterial
    block = 0
    prop_names  = 'M'
    prop_values = '3333.333'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-08
  l_max_its = 30

  dt = 0.0006
  num_steps = 8
[]

[Outputs]
  interval = 4
  exodus = true
[]
