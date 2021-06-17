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
  [eth_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [eth_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [eth_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [fth_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [fth_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [fth_zz]
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
  [eth_xx]
    type = RankTwoAux
    variable = eth_xx
    rank_two_tensor = thermal_eigenstrain
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [eth_yy]
    type = RankTwoAux
    variable = eth_yy
    rank_two_tensor = thermal_eigenstrain
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [eth_zz]
    type = RankTwoAux
    variable = eth_zz
    rank_two_tensor = thermal_eigenstrain
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [fth_xx]
    type = RankTwoAux
    variable = fth_xx
    rank_two_tensor = thermal_deformation_gradient
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [fth_yy]
    type = RankTwoAux
    variable = fth_yy
    rank_two_tensor = thermal_deformation_gradient
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [fth_zz]
    type = RankTwoAux
    variable = fth_zz
    rank_two_tensor = thermal_deformation_gradient
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
    type = ComputeElasticityTensorConstantRotationCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    eigenstrain_names = thermal_eigenstrain
    tan_mod_type = exact
    maximum_substep_iteration = 5
  []
  [trial_xtalpl]
    type = CrystalPlasticityKalidindiUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
  []
  [thermal_eigenstrain]
    type = ComputeCrystalPlasticityThermalEigenstrain
    eigenstrain_name = thermal_eigenstrain
    deformation_gradient_name = thermal_deformation_gradient
    temperature = temperature
    thermal_expansion_coefficients = '1e-05 2e-05 4e-05' # thermal expansion coefficients along three directions
  []
[]

[Postprocessors]
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [eth_xx]
    type = ElementAverageValue
    variable = eth_xx
  []
  [eth_yy]
    type = ElementAverageValue
    variable = eth_yy
  []
  [eth_zz]
    type = ElementAverageValue
    variable = eth_zz
  []
  [fth_xx]
    type = ElementAverageValue
    variable = fth_xx
  []
  [fth_yy]
    type = ElementAverageValue
    variable = fth_yy
  []
  [fth_zz]
    type = ElementAverageValue
    variable = fth_zz
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
