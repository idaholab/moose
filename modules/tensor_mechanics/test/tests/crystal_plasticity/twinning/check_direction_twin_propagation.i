[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    elem_type = HEX8
  []
[]

[AuxVariables]
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
  [twin_tau_0]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_1]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_2]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_3]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_4]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_5]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_6]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_7]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_8]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_9]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_10]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_tau_11]
   order = CONSTANT
   family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
[]

[AuxKernels]
  [twin_volume_fraction_0]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_0
   property = twin_system_volume_fraction
   index = 0
   execute_on = timestep_end
  []
  [twin_volume_fraction_1]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_1
   property = twin_system_volume_fraction
   index = 1
   execute_on = timestep_end
  []
  [twin_volume_fraction_2]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_2
   property = twin_system_volume_fraction
   index = 2
   execute_on = timestep_end
  []
  [twin_volume_fraction_3]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_3
   property = twin_system_volume_fraction
   index = 3
   execute_on = timestep_end
  []
  [twin_volume_fraction_4]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_4
   property = twin_system_volume_fraction
   index = 4
   execute_on = timestep_end
  []
  [twin_volume_fraction_5]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_5
   property = twin_system_volume_fraction
   index = 5
   execute_on = timestep_end
  []
  [twin_volume_fraction_6]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_6
   property = twin_system_volume_fraction
   index = 6
   execute_on = timestep_end
  []
  [twin_volume_fraction_7]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_7
   property = twin_system_volume_fraction
   index = 7
   execute_on = timestep_end
  []
  [twin_volume_fraction_8]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_8
   property = twin_system_volume_fraction
   index = 8
   execute_on = timestep_end
  []
  [twin_volume_fraction_9]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_9
   property = twin_system_volume_fraction
   index = 9
   execute_on = timestep_end
  []
  [twin_volume_fraction_10]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_10
   property = twin_system_volume_fraction
   index = 10
   execute_on = timestep_end
  []
  [twin_volume_fraction_11]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_11
   property = twin_system_volume_fraction
   index = 11
   execute_on = timestep_end
  []
  [twin_tau_0]
    type = MaterialStdVectorAux
    variable = twin_tau_0
    property = applied_shear_stress
    index = 0
    execute_on = timestep_end
  []
  [twin_tau_1]
    type = MaterialStdVectorAux
    variable = twin_tau_1
    property = applied_shear_stress
    index = 1
    execute_on = timestep_end
  []
  [twin_tau_2]
    type = MaterialStdVectorAux
    variable = twin_tau_2
    property = applied_shear_stress
    index = 2
    execute_on = timestep_end
  []
  [twin_tau_3]
    type = MaterialStdVectorAux
    variable = twin_tau_3
    property = applied_shear_stress
    index = 3
    execute_on = timestep_end
  []
  [twin_tau_4]
    type = MaterialStdVectorAux
    variable = twin_tau_4
    property = applied_shear_stress
    index = 4
    execute_on = timestep_end
  []
  [twin_tau_5]
    type = MaterialStdVectorAux
    variable = twin_tau_5
    property = applied_shear_stress
    index = 5
    execute_on = timestep_end
  []
  [twin_tau_6]
    type = MaterialStdVectorAux
    variable = twin_tau_6
    property = applied_shear_stress
    index = 6
    execute_on = timestep_end
  []
  [twin_tau_7]
    type = MaterialStdVectorAux
    variable = twin_tau_7
    property = applied_shear_stress
    index = 7
    execute_on = timestep_end
  []
  [twin_tau_8]
    type = MaterialStdVectorAux
    variable = twin_tau_8
    property = applied_shear_stress
    index = 8
    execute_on = timestep_end
  []
  [twin_tau_9]
    type = MaterialStdVectorAux
    variable = twin_tau_9
    property = applied_shear_stress
    index = 9
    execute_on = timestep_end
  []
  [twin_tau_10]
    type = MaterialStdVectorAux
    variable = twin_tau_10
    property = applied_shear_stress
    index = 10
    execute_on = timestep_end
  []
  [twin_tau_11]
    type = MaterialStdVectorAux
    variable = twin_tau_11
    property = applied_shear_stress
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
    function = '-5.0e-4*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorConstantRotationCP
    C_ijkl = '1.08e5 6.034e4 6.034e4 1.08e5 6.03e4 1.08e5 2.86e4 2.86e4 2.86e4' #Tallon and Wolfenden. J. Phys. Chem. Solids (1979)
    fill_method = symmetric9
    euler_angle_1 = 54.74
    euler_angle_2 = 45.0
    euler_angle_3 = 270.0
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'twin_only_xtalpl'
    tan_mod_type = exact
  []
  [twin_only_xtalpl]
    type = CrystalPlasticityTwinningKalidindiUpdate
    number_slip_systems = 12
    slip_sys_file_name = 'fcc_input_twinning_systems.txt'
    initial_twin_lattice_friction = 2.0
  []
[]

[Postprocessors]
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
  [twin_tau_0]
    type = ElementAverageValue
    variable = twin_tau_0
  []
  [twin_tau_1]
    type = ElementAverageValue
    variable = twin_tau_1
  []
  [twin_tau_2]
    type = ElementAverageValue
    variable = twin_tau_2
  []
  [twin_tau_3]
    type = ElementAverageValue
    variable = twin_tau_3
  []
  [twin_tau_4]
    type = ElementAverageValue
    variable = twin_tau_4
  []
  [twin_tau_5]
    type = ElementAverageValue
    variable = twin_tau_5
  []
  [twin_tau_6]
    type = ElementAverageValue
    variable = twin_tau_6
  []
  [twin_tau_7]
    type = ElementAverageValue
    variable = twin_tau_7
  []
  [twin_tau_8]
    type = ElementAverageValue
    variable = twin_tau_8
  []
  [twin_tau_9]
    type = ElementAverageValue
    variable = twin_tau_9
  []
  [twin_tau_10]
    type = ElementAverageValue
    variable = twin_tau_10
  []
  [twin_tau_11]
    type = ElementAverageValue
    variable = twin_tau_11
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

  dt = 0.025
  dtmin = 0.0125
  num_steps = 9
[]

[Outputs]
  csv = true
  perf_graph = true
[]
