[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = three_hexagons.e
  []
  patch_size = 10
  patch_update_strategy = auto
[]

[Functions]
  [pressure]
    type = PiecewiseLinear
    x = '0 10'
    y = '0 0.05'
    scale_factor = 1
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = true
    block = '1 2 3'
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = 'disp_x'
    boundary = '1001 1002 2001 2002 3001 3002'
    value = 0.0
  []
  [fix_y]
    type = DirichletBC
    variable = 'disp_y'
    boundary = '1001 1002 2001 2002 3001 3002'
    value = 0.0
  []
  [Pressure]
    [hex1_pressure]
      boundary = '110'
      function = pressure
      factor = 200
    []
  []
[]

[Contact]
  [contact_pressure1]
    formulation = penalty
    model = frictionless
    primary = 3333
    secondary = 1111
    penalty = 2e+03
    normalize_penalty = true
    normal_smoothing_distance = 0.2
    tangential_tolerance = 0.1
  []
  [contact_pressure2]
    formulation = penalty
    model = frictionless
    primary = 4444
    secondary = 2222
    penalty = 2e+03
    normalize_penalty = true
    normal_smoothing_distance = 0.2
    tangential_tolerance = 0.1
  []
  [contact_pressure3]
    formulation = penalty
    model = frictionless
    primary = 6666
    secondary = 5555
    penalty = 2e+03
    normalize_penalty = true
    normal_smoothing_distance = 0.2
    tangential_tolerance = 0.1
  []
[]

[Materials]
  [hex_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1 2 3'
    youngs_modulus = 1e4
    poissons_ratio = 0.0
  []
  [hex_strain]
    type = ComputePlaneFiniteStrain
    block = '1 2 3'
  []
  [hex_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2 3'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu       '

  line_search = 'basic'

  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-10

  l_max_its = 20
  dt = 0.5
  end_time = 1.5
[]

[Outputs]
  hide = 'penetration nodal_area'
  exodus = true
  perf_graph = true
[]
