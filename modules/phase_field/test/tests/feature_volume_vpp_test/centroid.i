[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 15
  nz = 0
  xmax = 50
  ymax = 25
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [c]
  []
  [w]
  []
  [eta]
  []
[]

[ICs]
  [rect_c]
    y2 = 20.0
    y1 = 5.0
    inside = 1.0
    x2 = 30.0
    variable = c
    x1 = 10.0
    type = BoundingBoxIC
  []
  [rect_eta]
    y2 = 20.0
    y1 = 5.0
    inside = 1.0
    x2 = 30.0
    variable = eta
    x1 = 10.0
    type = BoundingBoxIC
  []
[]

[Kernels]
  [c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
    coupled_variables = eta
  []
  [w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  []
  [time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  []

  [eta_dot]
    type = TimeDerivative
    variable = eta
  []
  [acint_eta]
    type = ACInterface
    variable = eta
    mob_name = M
    coupled_variables = c
    kappa_name = kappa_eta
  []
  [acbulk_eta]
    type = AllenCahn
    variable = eta
    mob_name = M
    f_name = F
    coupled_variables = c
  []
[]

[Materials]
  [pfmobility]
    type = GenericConstantMaterial
    prop_names = 'M    kappa_c  kappa_eta'
    prop_values = '5.0  2.0      0.1'
  []
  [free_energy]
    type = DerivativeParsedMaterial
    coupled_variables = 'c eta'
    constant_names = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    expression = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2+(c-eta)^2
    derivative_order = 2
  []
[]

[Postprocessors]
  [grain_center]
    type = GrainTracker
    variable = eta
    outputs = none
    compute_var_to_feature_map = true
    execute_on = 'timestep_begin'
  []

[]

[VectorPostprocessors]
  [grain_volumes]
    type = FeatureVolumeVectorPostprocessor
    flood_counter = grain_center
    execute_on = 'timestep_begin'
    output_centroids = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.2
  num_steps = 4
[]

[Outputs]
  csv = true
[]
