[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [blk1]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.10
    xmax = 0.75
    ymin = 0
    ymax = 2
    nx = 3
    ny = 3
    elem_type = QUAD4
    boundary_name_prefix = 'blk1'
    boundary_id_offset = 10
    subdomain_ids = '1'
    subdomain_name = 'blk1'
  []
  [blk2]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 2
    ymax = 3
    nx = 3
    ny = 3
    elem_type = QUAD4
    boundary_name_prefix = 'blk2'
    boundary_id_offset = 20
    subdomain_ids = '2'
    subdomain_name = 'blk2'
  []
  [hub_int]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.5
    xmax = 3.0
    ny = 1
    elem_type = EDGE2
    boundary_name_prefix = 'hub'
    boundary_id_offset = 910
    subdomain_ids = 910
    subdomain_name = 'hub'
  []
  [hub]
    type = TransformGenerator
    input = hub_int
    transform = TRANSLATE
    vector_value = '0 3 0'
  []
  [full]
    type = CombinerGenerator
    inputs = 'hub blk2 blk1'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      add_variables = false
      [all]
        new_system = true
        block = 'blk1 blk2'
      []
    []
  []
[]

[Materials]
  [stress]
    type = ComputeNeoHookeanStress
    # lambda and mu for young_modulus=5 and poissons_ratio=0.3
    lambda = '${fparse 5.0*0.3/(1+0.3)/(1-2*0.3)}'
    mu = '${fparse 5.0/2.0/(1+0.3)}'
    large_kinematics = true
    block = 'blk1 blk2'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Kernels]
  [hub_sd_0]
    type = NullKernel
    variable = disp_x
    block = hub
  []
  [hub_sd_1]
    type = NullKernel
    variable = disp_y
    block = hub
  []
[]

[Materials]
  # NullKernerl needs a place-hold material since other kernels have material
  [hub_null_mat]
    type = GenericConstantMaterial
    prop_names = null
    prop_values = 0.0
    block = hub
  []
[]

[BCs]
  # fix hub_right since we do not use it
  [fix_hub_end_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = hub_right
    value = 0
  []
  [fix_hub_end_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = hub_right
    value = 0
  []
[]

[Functions]
  [top_load]
    type = PiecewiseLinear
    x = '0.0  3.0'
    y = '0.0 -1.0'
  []
[]

[BCs]
  ## Fix the bottom of the BLK1 rubber block.
  [blk1_bottom_fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'blk1_bottom'
    value = 0
  []
  [blk1_bottom_fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'blk1_bottom'
    value = 0
  []

  ## hub top force
  [hub_top_force_y]
    type = FunctionNeumannBC
    variable = disp_y
    boundary = hub_left
    function = top_load
  []

  ##
  [blk_fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'blk1_left blk1_right blk2_left blk2_right hub_left'
    value = 0
  []
[]

[Constraints]
  [c_disp_x]
    type = LinearNodalConstraint
    variable = disp_x
    primary = '0'
    weights = 1
    secondary_node_set = 'blk2_top'
    penalty = 1e8
    formulation = penalty
  []
  [c_disp_y]
    type = LinearNodalConstraint
    variable = disp_y
    primary = '0'
    weights = 1
    secondary_node_set = 'blk2_top'
    penalty = 1e8
    formulation = penalty
  []
  [interface_x]
    type = TiedValueConstraint
    primary = blk2_bottom
    secondary = blk1_top
    variable = disp_x
    primary_variable = disp_x
  []
  [interface_y]
    type = TiedValueConstraint
    primary = blk2_bottom
    secondary = blk1_top
    primary_variable = disp_y
    variable = disp_y
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
  solve_type = NEWTON
  line_search = none
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  verbose = true
  nl_abs_tol = 1e-4
  l_tol = 1e-6
  nl_max_its = 50
  l_max_its = 40
  dt = 1
  dtmin = 0.1
  start_time = 0.0
  end_time = 3.0
[]

[Outputs]
  exodus = true
[]
