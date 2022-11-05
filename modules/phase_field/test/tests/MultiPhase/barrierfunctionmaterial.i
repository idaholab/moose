# This is a test of the BarrierFunctionMaterial option = HIGH

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
  elem_type = QUAD4
[]

[Variables]
  [./eta]
  [../]
[]

[ICs]
  [./IC_eta]
    type = SmoothCircleIC
    variable = eta
    x1 = 10
    y1 = 10
    radius = 5
    invalue = 1
    outvalue = 0
    int_width = 1
  [../]
[]

[Kernels]
  [./eta_bulk]
    type = AllenCahn
    variable = eta
    f_name = 0
    mob_name = 1
  [../]
  [./eta_interface]
    type = ACInterface
    variable = eta
    kappa_name = 1
    mob_name = 1
  [../]

  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[Materials]
  [./barrier]
    type = BarrierFunctionMaterial
    eta = eta
    g_order = HIGH
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 2
[]

[Problem]
  solve = false
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
