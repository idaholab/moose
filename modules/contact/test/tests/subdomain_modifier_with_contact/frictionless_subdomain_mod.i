[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  type = FEProblem
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Mesh]
  [gmg1]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 5
    ny = 5
    subdomain_ids = 1
  []
  [gmg2]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 0.9
    nx = 5
    ny = 5
    subdomain_ids = 2
  []
  [gmg3]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.5
    ymin = 0
    ymax = 0.5
    nx = 1
    ny = 1
    subdomain_ids = 3
  []
  [cmg]
    type = CombinerGenerator
    inputs = 'gmg1 gmg2 gmg3'
    positions = '0 -0.5 0 1.01 -0.45 0 3 -0.5 0'
  []
  # It would be nice to just use the original generated sidesets with prefixes (set using
  # 'boundary_name_prefix' in GeneratedMeshGenerator for the two combined blocks, but
  # CombinerGenerator didn't correctly preserve those sidesets for the two blocks when
  # this test was created. Consider doing that in the future.
  [ssg1]
    type = ParsedGenerateSideset
    input = cmg
    combinatorial_geometry = 'x < 0.0001'
    new_sideset_name = left_left
    include_only_external_sides = true
  []
  [ssg2]
    type = ParsedGenerateSideset
    input = ssg1
    combinatorial_geometry = 'x > 0.9999 & x < 1.0001'
    new_sideset_name = left_right
    include_only_external_sides = true
  []
  [ssg3]
    type = ParsedGenerateSideset
    input = ssg2
    combinatorial_geometry = 'x > 1.0099 & x < 1.01001'
    new_sideset_name = right_left
    include_only_external_sides = true
  []
  [ssg4]
    type = ParsedGenerateSideset
    input = ssg3
    combinatorial_geometry = 'x > 2.00999 & x < 2.01001'
    new_sideset_name = right_right
    include_only_external_sides = true
  []
[]

[GlobalParams]
  volumetric_locking_correction = false
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    block = '1 2'
  []
[]

[BCs]
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left_left
    value = 0.0
  []
  [left_y]
    type = DirichletBC
    variable = disp_y
    boundary = left_left
    value = 0.0
  []
  [right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right_right
    #Initial gap is 0.01, so this closes it
    function = -0.02*if(t<1,t,1)
  []
  [right_y]
    type = DirichletBC
    variable = disp_y
    boundary = right_right
    value = 0.0
  []
[]

[Materials]
  [left]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e7
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  []
[]

[MeshModifiers]
  [move_elems]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'a'
    criterion_type = 'ABOVE'
    threshold = 0.8
    subdomain_id = 3
    execute_on = 'TIMESTEP_BEGIN'
    block = 1
  []
[]

[AuxVariables]
  [a]
  []
[]

[AuxKernels]
  [a]
    type = ParsedAux
    expression = 'max(0,1-sqrt(x*x+y*y))*t'
    variable = a
    use_xyzt = true
  []
[]

[Contact]
  [leftright]
    primary = left_right
    secondary = right_left
    model = frictionless
    formulation = kinematic
    penalty = 1e9
    normal_smoothing_distance = 0.1
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'

  dt = 1.0
  dtmin = 1.0
  end_time = 2

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  l_max_its = 20
  nl_max_its = 20
[]

[Outputs]
  exodus = true
[]

