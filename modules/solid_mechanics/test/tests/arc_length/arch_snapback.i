radius = 2.39
half_angle = 24.7
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
  [pins]
    type = ExtraNodesetGenerator
    input = arch
    new_boundary = 'pins'
    coord = '${fparse radius * cos(angle_min * pi / 180)} ${fparse radius * sin(angle_min * pi / 180)} 0;
             ${fparse radius * cos(angle_max * pi / 180)} ${fparse radius * sin(angle_max * pi / 180)} 0'
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

[DiracKernels]
  [apex_load]
    type = ConstantPointSource
    variable = disp_y
    point = '0 ${radius} 0'
    value = -60
    vector_tags = 'arc_length_load'
  []
[]

[BCs]
  [pinned_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'pins'
    value = 0
  []
  [pinned_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'pins'
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
  lambda_max = 3
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
  # visualization of the deformed arch for the user, deliberately not asserted by any exodiff
  exodus = true
[]
