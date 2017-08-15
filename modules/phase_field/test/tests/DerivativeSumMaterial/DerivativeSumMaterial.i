[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 250
  ymin = 0
  ymax = 250
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 125.0
      y1 = 125.0
      radius = 80.0
      invalue = 1.0
      outvalue = 0.1
      int_width = 80.0
    [../]
  [../]
[]

[Kernels]
  [./w_res]
    type = Diffusion
    variable = c
  [../]
  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[Materials]
  [./free_energy1]
    type = DerivativeParsedMaterial
    f_name = Fa
    args = 'c'
    function = (c-0.1)^4*(1-0.1-c)^4
  [../]
  [./free_energy2]
    type = DerivativeParsedMaterial
    f_name = Fb
    args = 'c'
    function = -0.25*(c-0.1)^4*(1-0.1-c)^4
  [../]

  # Fa+Fb+Fb == Fc
  [./free_energy3]
    type = DerivativeParsedMaterial
    f_name = Fc
    args = 'c'
    function = 0.5*(c-0.1)^4*(1-0.1-c)^4
  [../]

  [./free_energy]
    type = DerivativeSumMaterial
    f_name = F
    sum_materials = 'Fa Fb Fb'
    args = 'c'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'NEWTON'

  l_max_its = 30
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  start_time = 0.0
  num_steps = 1

  dt = 0.1
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
