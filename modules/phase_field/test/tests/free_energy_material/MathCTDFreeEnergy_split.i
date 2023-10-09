[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmin = 0.0
  xmax = 30.0
  ymin = 0.0
  ymax = 30.0
  elem_type = QUAD4
[]

[Variables]
  [c]
    [InitialCondition]
      type = CrossIC
      x1 = 0.0
      x2 = 30.0
      y1 = 0.0
      y2 = 30.0
    []
  []
  [w]
  []
[]

[Kernels]
  [cres]
    type = SplitCHParsed
    variable = c
    kappa_name = 2.0 # kappa_c - we are not using a mat prop here to support AD+nonAD
    w = w
    f_name = F
  []

  [wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
  []

  [time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  []
[]

[BCs]
  [Periodic]
    [all]
      auto_direction = 'x y'
      variable = 'c w'
    []
  []
[]

[Materials]
  [constant]
    type = GenericConstantMaterial
    prop_names = 'M'
    prop_values = '1.0'
  []
  [free_energy]
    type = MathCTDFreeEnergy
    property_name = F
    c = c
  []
[]

[Executioner]
  type = Transient
  scheme = BDF2
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 30
  l_tol = 1.0e-3

  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 10.0
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
