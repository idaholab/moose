# this test is the same as frictionless_kinematic test but designed to test the gap offset capability
# gap offsets with value of 0.01 were introduced to both primary and secondary sides in the initial mesh
# these values were accounted using the gap offset capability to produce the same result as if no gap offsets were introduced
[Mesh]
  file = blocks_2d_gap_offset.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./inc_slip_x]
  [../]
  [./inc_slip_y]
  [../]
  [./accum_slip_x]
  [../]
  [./accum_slip_y]
  [../]
  [./primary_gap_offset]
  [../]
  [./secondary_gap_offset]
  [../]
  [./mapped_primary_gap_offset]
  [../]
[]

[Functions]
  [./vertical_movement]
    type = ParsedFunction
    expression = -t
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    strain = FINITE
  [../]
[]

[AuxKernels]
  [./zeroslip_x]
    type = ConstantAux
    variable = inc_slip_x
    boundary = 3
    execute_on = timestep_begin
    value = 0.0
  [../]
  [./zeroslip_y]
    type = ConstantAux
    variable = inc_slip_y
    boundary = 3
    execute_on = timestep_begin
    value = 0.0
  [../]
  [./accum_slip_x]
    type = AccumulateAux
    variable = accum_slip_x
    accumulate_from_variable = inc_slip_x
    execute_on = timestep_end
  [../]
  [./accum_slip_y]
    type = AccumulateAux
    variable = accum_slip_y
    accumulate_from_variable = inc_slip_y
    execute_on = timestep_end
  [../]
  [./primary_gap_offset]
    type = ConstantAux
    variable = primary_gap_offset
    value = -0.01
    boundary = 2
  [../]
  [./mapped_primary_gap_offset]
    type = GapValueAux
    variable = mapped_primary_gap_offset
    paired_variable = primary_gap_offset
    boundary = 3
    paired_boundary = 2
  [../]
  [./secondary_gap_offset]
    type = ConstantAux
    variable = secondary_gap_offset
    value = -0.01
    boundary = 3
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = -0.02
  [../]
  [./right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = vertical_movement
  [../]
[]

[Materials]
  [./left]
    type = ComputeIsotropicElasticityTensor
    block = 1
    poissons_ratio = 0.3
    youngs_modulus = 1e7
  [../]
  [./right]
    type = ComputeIsotropicElasticityTensor
    block = 2
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 1000
  dt = 0.01
  end_time = 0.10
  num_steps = 1000
  l_tol = 1e-6
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  dtmin = 0.01

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]
[]

[Outputs]
  file_base = frictionless_kinematic_gap_offsets_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
[]

[Contact]
  [./leftright]
    primary = 2
    secondary = 3
    model = frictionless
    penalty = 1e+6
    secondary_gap_offset = secondary_gap_offset
    mapped_primary_gap_offset = mapped_primary_gap_offset
  [../]
[]
