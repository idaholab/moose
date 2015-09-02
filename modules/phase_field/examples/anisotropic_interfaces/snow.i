[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmax = 9
  ymax = 9
  elem_type = QUAD4
[]

[Variables]
  [./w]
  [../]
  [./T]
  [../]
[]

[ICs]
  active = 'wIC TIC'
  [./wIC]
    type = SmoothCircleIC
    variable = w
    block = 0
    int_width = 0.01
    x1 = 4.5
    y1 = 4.5
    radius = 0.5
    outvalue = 0
    invalue = 1
    3D_spheres = false
  [../]
  [./TIC]
    type = ConstantIC
    variable = T
    block = 0
    value = 0
  [../]
[]

[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
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
    type = anisoACInterface1
    variable = w
    mob_name = M
  [../]
  [./anisoACinterface2]
    type = anisoACInterface2    
    variable = w
    mob_name = M
  [../]
  [./ACParsed]
    type = ACParsed
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
    type = CoefDiffusion
    variable = T
    coef = 1
  [../]
  [./w_dot_T]
    type = CoefCoupledTimeDerivative
    variable = T
    v = w
    coef = -1.8
  [../]
[]

[AuxKernels]
  [./local_energy]
    type = TotalFreeEnergy
    variable = local_energy
    f_name = fbulk
    interfacial_vars = w
    kappa_names = kappa_c
    execute_on = timestep_end
  [../]
  [./m]
    type = ParsedAux
    variable = m
    args = T
    function = 0.9*atan(10*(1-T))/3.14
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./Periodic]
  [../]
[]

[Materials]
  [./free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = fbulk
    args = 'w m'
    function = 1/4*w*w*w*w-(1/2-m/3)*w*w*w+(1/4-m/2)*w*w
    outputs = exodus
  [../]
  [./material]
    type = ExampleMaterial
    mob = 3333.333
    block = 0
    c = w
    kappa = 0.5
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient # Here we use the Transient Executioner
  dt = 0.002
  solve_type = NEWTON
  num_steps = 500
  line_search = none
[]

[Outputs]
   output_initial = true
   exodus = true
   print_linear_residuals = true
   print_perf_log = true 
[]

