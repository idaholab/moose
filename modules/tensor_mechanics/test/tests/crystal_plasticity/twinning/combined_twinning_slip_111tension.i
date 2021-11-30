[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = HEX8
  []
[]

[AuxVariables]
  [fp_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [total_twin_volume_fraction]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_0]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_3]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_4]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_5]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_6]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_7]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_8]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_9]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_10]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_volume_fraction_0]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_1]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_2]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_3]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_4]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_5]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_6]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_7]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_8]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_9]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_10]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_11]
   order = CONSTANT
   family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
[]

[AuxKernels]
  [fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = plastic_deformation_gradient
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [total_twin_volume_fraction]
    type = MaterialRealAux
    variable = total_twin_volume_fraction
    property = twin_total_volume_fraction_twins
    execute_on = timestep_end
  []
  [slip_increment_0]
   type = MaterialStdVectorAux
   variable = slip_increment_0
   property = slip_increment
   index = 0
   execute_on = timestep_end
  []
  [slip_increment_1]
   type = MaterialStdVectorAux
   variable = slip_increment_1
   property = slip_increment
   index = 1
   execute_on = timestep_end
  []
  [slip_increment_2]
   type = MaterialStdVectorAux
   variable = slip_increment_2
   property = slip_increment
   index = 2
   execute_on = timestep_end
  []
  [slip_increment_3]
   type = MaterialStdVectorAux
   variable = slip_increment_3
   property = slip_increment
   index = 3
   execute_on = timestep_end
  []
  [slip_increment_4]
   type = MaterialStdVectorAux
   variable = slip_increment_4
   property = slip_increment
   index = 4
   execute_on = timestep_end
  []
  [slip_increment_5]
   type = MaterialStdVectorAux
   variable = slip_increment_5
   property = slip_increment
   index = 5
   execute_on = timestep_end
  []
  [slip_increment_6]
   type = MaterialStdVectorAux
   variable = slip_increment_6
   property = slip_increment
   index = 6
   execute_on = timestep_end
  []
  [slip_increment_7]
   type = MaterialStdVectorAux
   variable = slip_increment_7
   property = slip_increment
   index = 7
   execute_on = timestep_end
  []
  [slip_increment_8]
   type = MaterialStdVectorAux
   variable = slip_increment_8
   property = slip_increment
   index = 8
   execute_on = timestep_end
  []
  [slip_increment_9]
   type = MaterialStdVectorAux
   variable = slip_increment_9
   property = slip_increment
   index = 9
   execute_on = timestep_end
  []
  [slip_increment_10]
   type = MaterialStdVectorAux
   variable = slip_increment_10
   property = slip_increment
   index = 10
   execute_on = timestep_end
  []
  [slip_increment_11]
   type = MaterialStdVectorAux
   variable = slip_increment_11
   property = slip_increment
   index = 11
   execute_on = timestep_end
  []
  [twin_volume_fraction_0]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_0
   property = twin_twin_system_volume_fraction
   index = 0
   execute_on = timestep_end
  []
  [twin_volume_fraction_1]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_1
   property = twin_twin_system_volume_fraction
   index = 1
   execute_on = timestep_end
  []
  [twin_volume_fraction_2]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_2
   property = twin_twin_system_volume_fraction
   index = 2
   execute_on = timestep_end
  []
  [twin_volume_fraction_3]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_3
   property = twin_twin_system_volume_fraction
   index = 3
   execute_on = timestep_end
  []
  [twin_volume_fraction_4]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_4
   property = twin_twin_system_volume_fraction
   index = 4
   execute_on = timestep_end
  []
  [twin_volume_fraction_5]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_5
   property = twin_twin_system_volume_fraction
   index = 5
   execute_on = timestep_end
  []
  [twin_volume_fraction_6]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_6
   property = twin_twin_system_volume_fraction
   index = 6
   execute_on = timestep_end
  []
  [twin_volume_fraction_7]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_7
   property = twin_twin_system_volume_fraction
   index = 7
   execute_on = timestep_end
  []
  [twin_volume_fraction_8]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_8
   property = twin_twin_system_volume_fraction
   index = 8
   execute_on = timestep_end
  []
  [twin_volume_fraction_9]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_9
   property = twin_twin_system_volume_fraction
   index = 9
   execute_on = timestep_end
  []
  [twin_volume_fraction_10]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_10
   property = twin_twin_system_volume_fraction
   index = 10
   execute_on = timestep_end
  []
  [twin_volume_fraction_11]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_11
   property = twin_twin_system_volume_fraction
   index = 11
   execute_on = timestep_end
  []
[]

[BCs]
  [fix_y]
    type = DirichletBC
    variable = disp_y
    preset = true
    boundary = 'bottom'
    value = 0
  []
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0
  []
  [tdisp]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = '0.02*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorConstantRotationCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5' # roughly copper
    fill_method = symmetric9
    euler_angle_1 = 54.74
    euler_angle_2 = 45.0
    euler_angle_3 = 270.0
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'twin_xtalpl slip_xtalpl'
    tan_mod_type = exact
  []
  [twin_xtalpl]
    type = CrystalPlasticityTwinningKalidindiUpdate
    base_name = twin
    number_slip_systems = 12
    slip_sys_file_name = 'fcc_input_twinning_systems.txt'
    initial_twin_lattice_friction = 60.0
  []
  [slip_xtalpl]
    type = CrystalPlasticityKalidindiUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
    total_twin_volume_fraction = 'twin_total_volume_fraction_twins'
  []
[]

[Postprocessors]
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  []
  [total_twin_volume_fraction]
    type = ElementAverageValue
    variable = total_twin_volume_fraction
  []
  [slip_increment_0]
    type = ElementAverageValue
    variable = slip_increment_0
  []
  [slip_increment_1]
    type = ElementAverageValue
    variable = slip_increment_1
  []
  [slip_increment_2]
    type = ElementAverageValue
    variable = slip_increment_2
  []
  [slip_increment_3]
    type = ElementAverageValue
    variable = slip_increment_3
  []
  [slip_increment_4]
    type = ElementAverageValue
    variable = slip_increment_4
  []
  [slip_increment_5]
    type = ElementAverageValue
    variable = slip_increment_5
  []
  [slip_increment_6]
    type = ElementAverageValue
    variable = slip_increment_6
  []
  [slip_increment_7]
    type = ElementAverageValue
    variable = slip_increment_7
  []
  [slip_increment_8]
    type = ElementAverageValue
    variable = slip_increment_8
  []
  [slip_increment_9]
    type = ElementAverageValue
    variable = slip_increment_9
  []
  [slip_increment_10]
    type = ElementAverageValue
    variable = slip_increment_10
  []
  [slip_increment_11]
    type = ElementAverageValue
    variable = slip_increment_11
  []
  [twin_volume_fraction_0]
    type = ElementAverageValue
    variable = twin_volume_fraction_0
  []
  [twin_volume_fraction_1]
    type = ElementAverageValue
    variable = twin_volume_fraction_1
  []
  [twin_volume_fraction_2]
    type = ElementAverageValue
    variable = twin_volume_fraction_2
  []
  [twin_volume_fraction_3]
    type = ElementAverageValue
    variable = twin_volume_fraction_3
  []
  [twin_volume_fraction_4]
    type = ElementAverageValue
    variable = twin_volume_fraction_4
  []
  [twin_volume_fraction_5]
    type = ElementAverageValue
    variable = twin_volume_fraction_5
  []
  [twin_volume_fraction_6]
    type = ElementAverageValue
    variable = twin_volume_fraction_6
  []
  [twin_volume_fraction_7]
    type = ElementAverageValue
    variable = twin_volume_fraction_7
  []
  [twin_volume_fraction_8]
    type = ElementAverageValue
    variable = twin_volume_fraction_8
  []
  [twin_volume_fraction_9]
    type = ElementAverageValue
    variable = twin_volume_fraction_9
  []
  [twin_volume_fraction_10]
    type = ElementAverageValue
    variable = twin_volume_fraction_10
  []
  [twin_volume_fraction_11]
    type = ElementAverageValue
    variable = twin_volume_fraction_11
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
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10

  dt = 0.005
  dtmin = 0.01
  num_steps = 6
[]

[Outputs]
  csv = true
  perf_graph = true
[]
