radius = 1.70
half_angle = 36
angle_min = ${fparse 90 - half_angle}
angle_max = ${fparse 90 + half_angle}

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [arch]
    type = AnnularMeshGenerator
    nt = 40
    nr = 4
    rmin = ${fparse radius - 0.02}
    rmax = ${fparse radius + 0.02}
    dmin = ${angle_min}
    dmax = ${angle_max}
  []
  [crown]
    type = ParsedGenerateSideset
    input = arch
    included_boundaries = 'rmax'
    combinatorial_geometry = 'abs(x) < 0.45'
    new_sideset_name = 'crown'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    decomposition_method = EigenSolution
    use_automatic_differentiation = true
    add_variables = true
  []
[]

[BCs]
  [crown_pressure_x]
    type = ADPressure
    variable = disp_x
    boundary = 'crown'
    factor = 67
    use_displaced_mesh = true
    vector_tags = 'arc_length_load'
    matrix_tags = 'arc_length_load_jac'
  []
  [crown_pressure_y]
    type = ADPressure
    variable = disp_y
    boundary = 'crown'
    factor = 67
    use_displaced_mesh = true
    vector_tags = 'arc_length_load'
    matrix_tags = 'arc_length_load_jac'
  []
  [clamped_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'dmin dmax'
    value = 0
  []
  [clamped_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'dmin dmax'
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[Problem]
  type = ArcLengthProblem
  step_size = 0.06
  psi_squared = 0
  lambda_min = -4
  lambda_max = 5
  max_continuation_steps = 300
[]

[Postprocessors]
  [apex_uy]
    type = PointValue
    variable = disp_y
    point = '0 ${radius} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [lambda]
    type = ArcLengthLoadParameter
  []
[]

[VectorPostprocessors]
  [path]
    type = ArcLengthHistory
    postprocessors = 'apex_uy'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]
