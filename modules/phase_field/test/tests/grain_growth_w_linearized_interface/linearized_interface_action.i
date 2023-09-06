[GlobalParams]
  bound_value = 5.0
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 50
  ymax = 50
  nx = 10
  ny = 10
[]

[Modules]
  [PhaseField]
    [GrainGrowthLinearizedInterface]
      op_num = 2
      var_name_base = phi
      op_name_base = gr
      mobility = L
      kappa = kappa_op
    []
  []
[]

[ICs]
  [phi0_IC]
    type = SmoothCircleICLinearizedInterface
    variable = phi0
    invalue = 1.0
    outvalue = 0.0
    radius = 30
    int_width = 10
    x1 = 0.0
    y1 = 0.0
    profile = TANH
  []
  [phi1_IC]
    type = SmoothCircleICLinearizedInterface
    variable = phi1
    invalue = 0.0
    outvalue = 1.0
    radius = 30
    int_width = 10
    x1 = 0.0
    y1 = 0.0
    profile = TANH
  []
[]

[Materials]
  [GBEovlution]
    type = GBEvolution
    GBenergy = 0.97
    GBMobility = 0.6e-6
    T = 300
    wGB = 10
  []
[]

[Postprocessors]
  [grain_area_mat]
    type = ElementIntegralMaterialProperty
    mat_prop = gr0
    execute_on = 'initial TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON

  petsc_options_iname = '-pc_type -ksp_type -snes_type'
  petsc_options_value = 'bjacobi gmres vinewtonrsls'

  dt = 0.1
  end_time = 0.6
[]

[Outputs]
  exodus = true
[]
