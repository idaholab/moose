[Mesh]
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    ymin = -1
    ny = 2
    boundary_name_prefix = '1'
  []
  [right_block_sidesets]
    type = RenameBoundaryGenerator
    input = right_block
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block_sidesets
    subdomain_id = 1
  []

  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = -0.5
    boundary_name_prefix = '2'
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = '20 21 22 23'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 2
  []

  [combined_mesh]
    type = MeshCollectionGenerator
    inputs = 'right_block_id left_block_id'
  []
[]

[Variables]
  [temperature]
    initial_condition = 298
  []
[]

[Kernels]
  [Tdot]
    type = TimeDerivative
    variable = temperature
  []
  [heat_conduction]
    type = Diffusion
    variable = temperature
  []
[]

[UserObjects]
  [w_complement_mvg_bnd]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'temperature'
    block = '1'
    criterion_type = ABOVE
    threshold = 400
    subdomain_id = 2
    complement_moving_boundary_name = 10
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[BCs]
  [tempBC]
    type = DirichletBC
    variable = temperature
    boundary = '12'
    value = 300
  []
  [fluxBC]
    type = NeumannBC
    variable = temperature
    boundary = '10'
    value = '100'
  []
[]

[Postprocessors]
  [temp_top_element]
    type = PointValue
    variable = temperature
    point = '0 0.5 0'
  []
[]

[Executioner]
  type = Transient
  end_time = 5
  dtmin = 1
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew -snes_converged_reason'
  petsc_options_iname = '-pc_type -pc_f./moactor_mat_solver_package -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu superlu_dist NONZERO 1000'
  line_search = none

[]

[Outputs]
  exodus = true
[]
