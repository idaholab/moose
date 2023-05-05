[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = three_hexagons_coarse.e
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

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    block = '1 2 3'
    planar_formulation = PLANE_STRAIN
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
      factor = 80
    []
    [hex2_pressure]
      boundary = '210'
      function = pressure
      factor = 50
    []
  []
[]

[Contact]
  [contact_pressure]
    formulation = penalty
    model = frictionless
    penalty = 2e+03
    normalize_penalty = true
    automatic_pairing_distance = 2.75
  []
[]

[Materials]
  [hex_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1 2 3'
    youngs_modulus = 1e4
    poissons_ratio = 0.0
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

  line_search = 'none'

  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-10

  l_max_its = 20
  dt = 0.5
  end_time = 4.0
[]

[Outputs]
  exodus = true
[]
