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
  [total_twin_volume_fraction]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_0]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_3]
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
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
  generate_output = stress_zz
[]

[AuxKernels]
  [total_twin_volume_fraction]
    type = MaterialRealAux
    variable = total_twin_volume_fraction
    property = total_volume_fraction_twins
    execute_on = timestep_end
  []
  [twin_resistance_0]
   type = MaterialStdVectorAux
   variable = twin_resistance_0
   property = slip_resistance
   index = 0
   execute_on = timestep_end
  []
  [twin_resistance_1]
   type = MaterialStdVectorAux
   variable = twin_resistance_1
   property = slip_resistance
   index = 1
   execute_on = timestep_end
  []
  [twin_resistance_2]
   type = MaterialStdVectorAux
   variable = twin_resistance_2
   property = slip_resistance
   index = 2
   execute_on = timestep_end
  []
  [twin_resistance_3]
   type = MaterialStdVectorAux
   variable = twin_resistance_3
   property = slip_resistance
   index = 3
   execute_on = timestep_end
  []
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
    function = '-1.0e-3*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.08e5 6.034e4 6.034e4 1.08e5 6.03e4 1.08e5 2.86e4 2.86e4 2.86e4' #Tallon and Wolfenden. J. Phys. Chem. Solids (1979)
    fill_method = symmetric9
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'twin_only_xtalpl'
    tan_mod_type = exact
  []
  [twin_only_xtalpl]
    type = CrystalPlasticityTwinningKalidindiUpdate
    number_slip_systems = 4
    slip_sys_file_name = 'select_twin_systems_verify_hardening.txt'
    initial_twin_lattice_friction = 6.0
    non_coplanar_coefficient_twin_hardening = 0
    non_coplanar_twin_hardening_exponent = 0
    coplanar_coefficient_twin_hardening = 8e8
  []
[]

[Postprocessors]
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [total_twin_volume_fraction]
    type = ElementAverageValue
    variable = total_twin_volume_fraction
  []
  [twin_resistance_0]
    type = ElementAverageValue
    variable = twin_resistance_0
  []
  [twin_resistance_1]
    type = ElementAverageValue
    variable = twin_resistance_1
  []
  [twin_resistance_2]
    type = ElementAverageValue
    variable = twin_resistance_2
  []
  [twin_resistance_3]
    type = ElementAverageValue
    variable = twin_resistance_3
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

  dt = 0.05
  dtmin = 1e-6
  dtmax = 10.0
  num_steps = 4
[]

[Outputs]
  csv = true
  perf_graph = true
[]
