# This test is documented as an example for ConstrainedShapeOptimization. This
# test should not be changed without updating the documentation.

inner_radius = 6
outer_radius = 10
volume_constraint = 200

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [mesh]
    type = ConcentricCircleMeshGenerator
    has_outer_square = no
    num_sectors = 16
    radii = '${inner_radius} ${outer_radius}'
    rings = '16 16'
    preserve_volumes = false
  []
  [inner_radius]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    new_boundary = inner
    primary_block = 2
    paired_block = 1
  []

  [delete]
    type = BlockDeletionGenerator
    input = inner_radius
    block = 1
  []
  [gather_all]
    type = BoundingBoxNodeSetGenerator
    input = delete
    bottom_left = '-100 -100 -100'
    top_right = '100 100 100'
    new_boundary = total
  []
  [combine]
    type = SideSetsFromBoundingBoxGenerator
    input = gather_all
    bottom_left = '-100 -100 -100'
    top_right = '100 100 100'
    boundaries_old = 'inner outer'
    boundary_new = moving
  []

[]
[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [T]
  []
[]

[AuxVariables]
  [dist_between]
    [AuxKernel]
      type = NearestNodeDistanceAux
      variable = dist_between
      paired_boundary = moving
      boundary = total
      block = 2
      use_displaced_mesh = false
      execute_on = "INITIAL"
    []
  []
[]

[Kernels]
  [disp_x]
    type = MatDiffusion
    variable = disp_x
    use_displaced_mesh = false
    diffusivity = diff_coef
  []
  [disp_y]
    type = MatDiffusion
    variable = disp_y
    use_displaced_mesh = false
    diffusivity = diff_coef
  []
  # run physics of interest on deformed mesh
  [Diffusion]
    type = FunctionDiffusion
    variable = T
    use_displaced_mesh = true
  []
  [Source]
    type = BodyForce
    variable = T
    value = 1
    use_displaced_mesh = true
  []
[]

[Materials]
  # perserve elements near the boundary
  [diff_coef]
    type = ParsedMaterial
    coupled_variables = 'dist_between'
    expression = '1/(dist_between+0.5)'
    property_name = 'diff_coef'
  []
  [h]
    type = ADGenericFunctionMaterial
    prop_names = h
    prop_values = h
  []
  # convection type boundary condition
  [convection_bc]
    type = ADParsedMaterial
    coupled_variables = "T"
    expression = "h*(100-T)"
    material_property_names = "h"
    property_name = convection
  []
[]

[Functions]
  [r1_x]
    type = ParsedOptimizationFunction
    expression = 'r1 * cos((atan(y/x)))'
    param_symbol_names = 'r0 r1'
    param_vector_name = 'params/radii'
  []
  [r1_y]
    type = ParsedOptimizationFunction
    expression = 'r1 * sin((atan(y/x)))'
    param_symbol_names = 'r0 r1'
    param_vector_name = 'params/radii'
  []
  [r0_x]
    type = ParsedOptimizationFunction
    expression = 'r0 * cos((atan(y/x)))'
    param_symbol_names = 'r0 r1'
    param_vector_name = 'params/radii'
  []
  [r0_y]
    type = ParsedOptimizationFunction
    expression = 'r0 * sin((atan(y/x)))'
    param_symbol_names = 'r0 r1'
    param_vector_name = 'params/radii'
  []
  [h]
    type = ParsedOptimizationFunction
    # r0+${inner_radius} is the true current inner radius
    expression = '10 /(pi * (r0+${inner_radius})^3)'
    param_symbol_names = 'r0 r1'
    param_vector_name = 'params/radii'
  []
  [eq_grad_r0]
    type = ParsedOptimizationFunction
    expression = '-2 * pi * (r0 + ${inner_radius})'
    param_symbol_names = 'r0 r1'
    param_vector_name = 'params/radii'
  []
  [eq_grad_r1]
    type = ParsedOptimizationFunction
    # r1+${outer_radius} is the true current outer radius
    expression = '2 * pi * (r1+${outer_radius})'
    param_symbol_names = 'r0 r1'
    param_vector_name = 'params/radii'
  []
[]

[BCs]
  [diffuse_r1_x]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = 'outer'
    function = r1_x
    preset = false
  []
  [diffuse_r1_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 'outer'
    function = r1_y
    preset = false
  []
  [diffuse_r0_x]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = 'inner'
    function = r0_x
    preset = false
  []
  [diffuse_r0_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 'inner'
    function = r0_y
    preset = false
  []
  # run physics on deformed mesh
  [convection]
    type = ADMatNeumannBC
    variable = T
    boundary = inner
    boundary_material = convection
    use_displaced_mesh = true
    value = 1
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Reporters]
  [params]
    type = ConstantReporter
    real_vector_names = 'radii'
    real_vector_values = '0 0'
    dof_id_type_vector_names = 'num_params'
    dof_id_type_vector_values = '2'
  []
[]

[Postprocessors]
  [current_volume]
    type = VolumePostprocessor
    use_displaced_mesh = true
  []
  # objective function
  [objective]
    type = NodalExtremeValue
    variable = T
  []
  [eq_constraint]
    type = ParsedPostprocessor
    pp_names = current_volume
    expression = 'current_volume - ${volume_constraint}'
  []
  [func_r0]
    type = FunctionValuePostprocessor
    function = eq_grad_r0
  []
  [func_r1]
    type = FunctionValuePostprocessor
    function = eq_grad_r1
  []
[]

[VectorPostprocessors]
  # convert "Real" postprocessors to vectors
  [vol_constraint]
    type = VectorOfPostprocessors
    postprocessors = 'eq_constraint'
    force_postaux = true
  []
  [eq_grad]
    type = VectorOfPostprocessors
    postprocessors = 'func_r0 func_r1'
    force_postaux = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = none
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
[]

[Outputs]
  console = false
[]

