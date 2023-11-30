[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
[]

[AuxVariables]
  [temperature]
    order = FIRST
    family = LAGRANGE
  []
  [f1_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [f1_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [f1_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [f2_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [f2_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [f2_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [feig_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [feig_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [feig_zz]
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
  [temperature]
    type = FunctionAux
    variable = temperature
    function = '300+400*t' # temperature increases at a constant rate
    execute_on = timestep_begin
  []
  [f1_xx]
    type = RankTwoAux
    variable = f1_xx
    rank_two_tensor = thermal_deformation_gradient_1
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [f1_yy]
    type = RankTwoAux
    variable = f1_yy
    rank_two_tensor = thermal_deformation_gradient_1
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [f1_zz]
    type = RankTwoAux
    variable = f1_zz
    rank_two_tensor = thermal_deformation_gradient_1
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [f2_xx]
    type = RankTwoAux
    variable = f2_xx
    rank_two_tensor = thermal_deformation_gradient_2
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [f2_yy]
    type = RankTwoAux
    variable = f2_yy
    rank_two_tensor = thermal_deformation_gradient_2
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [f2_zz]
    type = RankTwoAux
    variable = f2_zz
    rank_two_tensor = thermal_deformation_gradient_2
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [feig_xx]
    type = RankTwoAux
    variable = feig_xx
    rank_two_tensor = eigenstrain_deformation_gradient
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [feig_yy]
    type = RankTwoAux
    variable = feig_yy
    rank_two_tensor = eigenstrain_deformation_gradient
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [feig_zz]
    type = RankTwoAux
    variable = feig_zz
    rank_two_tensor = eigenstrain_deformation_gradient
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [tdisp]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    eigenstrain_names = "thermal_eigenstrain_1 thermal_eigenstrain_2"
    tan_mod_type = exact
    maximum_substep_iteration = 5
  []
  [trial_xtalpl]
    type = CrystalPlasticityKalidindiUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
  []
  [thermal_eigenstrain_1]
    type = ComputeCrystalPlasticityThermalEigenstrain
    eigenstrain_name = thermal_eigenstrain_1
    deformation_gradient_name = thermal_deformation_gradient_1
    temperature = temperature
    thermal_expansion_coefficients = '1e-05 2e-05 3e-05' # thermal expansion coefficients along three directions
  []
  [thermal_eigenstrain_2]
    type = ComputeCrystalPlasticityThermalEigenstrain
    eigenstrain_name = thermal_eigenstrain_2
    deformation_gradient_name = thermal_deformation_gradient_2
    temperature = temperature
    thermal_expansion_coefficients = '2e-05 3e-05 4e-05' # thermal expansion coefficients along three directions
  []
[]

[Postprocessors]
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [f1_xx]
    type = ElementAverageValue
    variable = f1_xx
  []
  [f1_yy]
    type = ElementAverageValue
    variable = f1_yy
  []
  [f1_zz]
    type = ElementAverageValue
    variable = f1_zz
  []
  [f2_xx]
    type = ElementAverageValue
    variable = f2_xx
  []
  [f2_yy]
    type = ElementAverageValue
    variable = f2_yy
  []
  [f2_zz]
    type = ElementAverageValue
    variable = f2_zz
  []
  [feig_xx]
    type = ElementAverageValue
    variable = feig_xx
  []
  [feig_yy]
    type = ElementAverageValue
    variable = feig_yy
  []
  [feig_zz]
    type = ElementAverageValue
    variable = feig_zz
  []
  [temperature]
    type = ElementAverageValue
    variable = temperature
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

  dt = 0.1
  dtmin = 1e-4
  end_time = 10
[]

[Outputs]
  csv = true
  [console]
    type = Console
    max_rows = 5
  []
[]
