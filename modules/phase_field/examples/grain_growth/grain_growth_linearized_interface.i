[GlobalParams]
  bound_value = 5.0
  op_num = 8
  var_name_base = phi
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 1000
  ymax = 1000
  nx = 100
  ny = 100
  uniform_refine = 1
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
      nonlinear_preconditioning = true
    []
  []
[]

[UserObjects]
  [RandomVoronoi]
    type = PolycrystalVoronoi
    grain_num = 60
    int_width = 10
    rand_seed = 103838
  []
  [grain_tracker]
    type = GrainTracker
    threshold = -4.0
    compute_halo_maps = true # Only necessary for displaying HALOS
  []
[]

[AuxVariables]
  [unique_grains]
    order = CONSTANT
    family = MONOMIAL
  []
  [var_indices]
    order = CONSTANT
    family = MONOMIAL
  []
  [halos]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
    execute_on = 'initial timestep_end'
  []
  [var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
    execute_on = 'initial timestep_end'
  []
  [halos]
    type = FeatureFloodCountAux
    variable = halos
    flood_counter = grain_tracker
    field_display = HALOS
    execute_on = 'initial timestep_end'
  []
[]

[Materials]
  [properties]
    type = GenericConstantMaterial
    prop_names = 'gbmob gbenergy gbwidth gamma_asymm'
    prop_values = '100 6 10 1.5'
  []
  [kappa_op]
    type = ParsedMaterial
    material_property_names = 'gbenergy gbwidth'
    property_name = kappa_op
    expression = '3/4*gbenergy*gbwidth'
  []
  [L]
    type = ParsedMaterial
    material_property_names = 'gbmob gbwidth'
    property_name = L
    expression = '4/3*gbmob/gbwidth'
  []
  [mu]
    type = ParsedMaterial
    material_property_names = 'gbenergy gbwidth'
    property_name = mu
    expression = '6*gbenergy/gbwidth'
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
    execute_on = 'initial TIMESTEP_END'
  []
[]

[BCs]
  [Periodic]
    [All]
      auto_direction = 'x y'
    []
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK

  petsc_options_iname = '-pc_type -pc_hypre_type -snes_type'
  petsc_options_value = 'hypre    boomeramg      vinewtonrsls'

  l_tol = 1e-4
  nl_max_its = 10
  l_max_its = 45

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.02
    optimal_iterations = 6
  []
  end_time = 30
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
