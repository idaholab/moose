[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 4
    nz = 2
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 2.0
    zmin = 0.0
    zmax = 1.0
    elem_type = HEX8
  []
  [rotate]
    type = TransformGenerator
    transform = ROTATE
    vector_value = '0 -20 -60'
    input = generated_mesh
  []
[]

[Physics/SolidMechanics/QuasiStatic/All]
  strain = FINITE
  add_variables = true
[]

[BCs]
  [Pressure]
    [top]
      boundary = top
      function = '-1000*t'
    []
  []
[]

[Constraints]
  [right]
    type = InclinedNoDisplacementConstraint
    boundary = right
    displacements = 'disp_x disp_y disp_z'
  []
  [bottom]
    type = InclinedNoDisplacementConstraint
    boundary = bottom
    displacements = 'disp_x disp_y disp_z'
  []
  [back]
    type = InclinedNoDisplacementConstraint
    boundary = back
    displacements = 'disp_x disp_y disp_z'
  []
[]

[AuxVariables]
  [un_right]
    order = FIRST
    family = LAGRANGE
  []
  [un_bottom]
    order = FIRST
    family = LAGRANGE
  []
  [un_back]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [un_right]
    type = ParsedAux
    variable = un_right
    boundary = right
    coupled_variables = 'disp_x disp_y disp_z'
    constant_names = 'nx ny nz'
    constant_expressions = 'sin(30*acos(-1)/180) -cos(30*acos(-1)/180) 0'
    expression = 'disp_x * nx + disp_y * ny + disp_z * nz'
    execute_on = 'TIMESTEP_END'
  []
  [un_bottom]
    type = ParsedAux
    variable = un_bottom
    boundary = bottom
    coupled_variables = 'disp_x disp_y disp_z'
    constant_names = 'nx ny nz'
    constant_expressions = 'cos(20*acos(-1)/180)*cos(30*acos(-1)/180) cos(20*acos(-1)/180)*sin(30*acos(-1)/180) -sin(20*acos(-1)/180)'
    expression = 'disp_x * nx + disp_y * ny + disp_z * nz'
    execute_on = 'TIMESTEP_END'
  []
  [un_back]
    type = ParsedAux
    variable = un_back
    boundary = back
    coupled_variables = 'disp_x disp_y disp_z'
    constant_names = 'nx ny nz'
    constant_expressions = 'sin(20*acos(-1)/180)*cos(30*acos(-1)/180) sin(20*acos(-1)/180)*sin(30*acos(-1)/180) cos(20*acos(-1)/180)'
    expression = 'disp_x * nx + disp_y * ny + disp_z * nz'
    execute_on = 'TIMESTEP_END'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  # controls for linear iterations
  l_max_its = 10
  l_tol = 1e-4

  # controls for nonlinear iterations
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12

  # time control
  start_time = 0.0
  dt = 1
  end_time = 5
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [un_max_right]
    type = NodalExtremeValue
    variable = un_right
    boundary = right
    value_type = max_abs
  []
  [un_max_bottom]
    type = NodalExtremeValue
    variable = un_bottom
    boundary = bottom
    value_type = max_abs
  []
  [un_max_back]
    type = NodalExtremeValue
    variable = un_back
    boundary = back
    value_type = max_abs
  []
  [un_max]
    type = ParsedPostprocessor
    pp_names = 'un_max_right un_max_bottom un_max_back'
    expression = 'max(max(abs(un_max_right), abs(un_max_bottom)), abs(un_max_back))'
  []
[]

[Outputs]
  [exo]
    type = Exodus
    file_base = inclined_bc_3d_out
    show = 'disp_x disp_y disp_z'
  []
  [csv]
    type = CSV
    file_base = inclined_rows_3d_out
    show = 'un_max'
  []
[]
