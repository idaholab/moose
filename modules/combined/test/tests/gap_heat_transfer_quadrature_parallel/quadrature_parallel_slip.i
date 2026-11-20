# Quadrature thermal contact gap distances must stay current in parallel when kinematic mechanical
# contact shares the surface pair. The left block slides up past the fixed right block while the
# gap closes uniformly, so every quadrature point on left_right reports the same gap distance at
# each step and distance_spread stays zero on any number of processors.
#
# distance_spread becomes nonzero in parallel if MooseMesh::possiblyRebuildNodeToElemMap drops the
# node-to-element entries of existing quadrature nodes. The mechanical contact constraint makes the
# first geometric search, which creates the quadrature nodes, run before the displaced mesh's
# node-to-element map is rebuilt. A quadrature node without an entry is then tracked only while its
# patch reaches the processor that owns it, so near a processor boundary it stops being updated
# and keeps the distance from the first step while the others follow the closing gap.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [left]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ny = 20
    boundary_name_prefix = left
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1.1
    xmax = 2
    ymin = -1
    ymax = 2
    ny = 60
    subdomain_ids = 1
    boundary_name_prefix = right
    boundary_id_offset = 10
  []
  [combine]
    type = CombinerGenerator
    inputs = 'left right'
  []
  patch_size = 1
  patch_update_strategy = always
  partitioner = centroid
  centroid_partitioner_direction = y
[]

[Variables]
  [T]
  []
[]

[Functions]
  [left_disp_x]
    type = ParsedFunction
    expression = '0.05 * t'
  []
  [left_disp_y]
    type = ParsedFunction
    expression = '0.5 * t'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = T
  []
[]

[Contact]
  [mechanical]
    primary = right_left
    secondary = left_right
    formulation = kinematic
    model = frictionless
    penalty = 1e6
  []
[]

[ThermalContact]
  [thermal]
    type = GapHeatTransfer
    variable = T
    primary = right_left
    secondary = left_right
    gap_conductivity = 1
    quadrature = true
  []
[]

[BCs]
  [left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'left_left left_bottom left_top'
    function = left_disp_x
  []
  [left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'left_left left_bottom left_top'
    function = left_disp_y
  []
  [right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right_right right_bottom right_top'
    value = 0
  []
  [right_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right_right right_bottom right_top'
    value = 0
  []
  [T_left]
    type = DirichletBC
    variable = T
    boundary = left_left
    value = 1
  []
  [T_right]
    type = DirichletBC
    variable = T
    boundary = right_right
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [thermal]
    type = HeatConductionMaterial
    thermal_conductivity = 1
  []
[]

[Postprocessors]
  [max_distance]
    type = SideExtremeValue
    variable = qpoint_penetration
    boundary = left_right
    value_type = max
  []
  [min_distance]
    type = SideExtremeValue
    variable = qpoint_penetration
    boundary = left_right
    value_type = min
  []
  [distance_spread]
    type = ParsedPostprocessor
    expression = 'max_distance - min_distance'
    pp_names = 'max_distance min_distance'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  line_search = none
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  csv = true
[]
