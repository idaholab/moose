[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  use_displaced_mesh = false
  [file]
    type = FileMeshGenerator
    file = one_duct.e
  []
[]

[Functions]
  [pressure]
    type = PiecewiseLinear
    x = '0 10'
    y = '0 0.005'
    scale_factor = 1
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    block = '1'
  []
[]

[BCs]
  [fix_y]
    type = DirichletBC
    variable = 'disp_y'
    boundary = '1001'
    value = 0.0
  []
  [fix_x]
    type = DirichletBC
    variable = 'disp_x'
    boundary = '16'
    value = 0.0
  []
  [fix_z]
    type = DirichletBC
    variable = 'disp_z'
    boundary = '16'
    value = 0.0
  []
  [Pressure]
    [hex1_pressure]
      boundary = '4'
      function = pressure
      factor = 80
    []
  []
[]

[VectorPostprocessors]
  [section_output]
    type = AverageSectionValueSampler
    axis_direction = '0 0 1'
    block = '1'
    variables = 'disp_x disp_y disp_z'
    reference_point = '0 0 0'
    require_equal_node_counts = false
  []
[]

[Adaptivity]
  steps = 1
  marker = box
  max_h_level = 2
  interval = 1
  [Markers]
    [box]
      type = BoxMarker
      bottom_left = '-2 -2 17.5'
      top_right = '2 2 21'
      inside = refine
      outside = do_nothing
    []
  []
[]

[Materials]
  [hex_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e4
    poissons_ratio = 0.0
  []
  [hex_stress]
    type = ComputeLinearElasticStress
    block = '1'
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
  end_time = 1.0
[]

[Outputs]
  exodus = true
  csv = true
[]
