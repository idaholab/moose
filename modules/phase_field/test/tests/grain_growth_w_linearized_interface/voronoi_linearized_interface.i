[GlobalParams]
  bound_value = 5.0
  op_num = 5
  var_name_base = phi
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 100
  ymax = 100
  nx = 20
  ny = 20
[]

[Modules]
  [PhaseField]
    [GrainGrowthLinearizedInterface]
      op_name_base = gr
      mobility = L
      kappa = kappa_op
    []
  []
[]

[ICs]
  [PolycrystalICs]
    [PolycrystalColoringIC]
      polycrystal_ic_uo = RandomVoronoi
      linearized_interface = true
    []
  []
[]

[UserObjects]
  [RandomVoronoi]
    type = PolycrystalVoronoi
    grain_num = 5
    int_width = 10
    rand_seed = 103838
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

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON

  petsc_options_iname = '-pc_type -ksp_type -snes_type'
  petsc_options_value = 'bjacobi gmres vinewtonrsls'

  dt = 0.05
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
