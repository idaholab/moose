[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmax = 4
  nx = 4
  elem_type = HEX8
[]

[AuxVariables]
  [euler_angle_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [euler_angle_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [euler_angle_3]
    order = CONSTANT
    family = MONOMIAL
  []
  # Euler angles aux variable to check the correctness of value assignments
  [check_euler_angle_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [check_euler_angle_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [check_euler_angle_3]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        add_variables = true
        generate_output = stress_zz
      []
    []
  []
[]

[AuxKernels]
  [euler_angle_1]
    type = FunctionAux
    variable = euler_angle_1
    function = '10*t'
  []
  [euler_angle_2]
    type = FunctionAux
    variable = euler_angle_2
    function = '20*t'
  []
  [euler_angle_3]
    type = FunctionAux
    variable = euler_angle_3
    function = '30*t'
  []
  # output Euler angles material property to check correctness of value assignment
  [mat_euler_angle_1]
    type = MaterialRealVectorValueAux
    variable = check_euler_angle_1
    property = 'Euler_angles'
    component = 0
   []
   [mat_euler_angle_2]
    type = MaterialRealVectorValueAux
    variable = check_euler_angle_2
    property = 'Euler_angles'
    component = 1
   []
   [mat_euler_angle_3]
    type = MaterialRealVectorValueAux
    variable = check_euler_angle_3
    property = 'Euler_angles'
    component = 2
   []
[]

[BCs]
  [Periodic]
    [all]
      variable = 'disp_x'
      auto_direction = 'z'
    []
  []
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
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
    boundary = 'front'
    function = '0.01*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
    read_prop_user_object = prop_read
    euler_angle_variables = 'euler_angle_1 euler_angle_2 euler_angle_3'
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    tan_mod_type = exact
  []
  [trial_xtalpl]
    type = CrystalPlasticityKalidindiUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
  []
[]

[UserObjects]
  [prop_read]
    type = PropertyReadFile
    prop_file_name = 'euler_ang_file.txt'
    # Enter file data as prop#1, prop#2, .., prop#nprop
    nprop = 3
    read_type = element
  []
[]

[Postprocessors]
  [check_euler_angle_1]
    type = ElementAverageValue
    variable = check_euler_angle_1
  []
  [check_euler_angle_2]
    type = ElementAverageValue
    variable = check_euler_angle_2
  []
  [check_euler_angle_3]
    type = ElementAverageValue
    variable = check_euler_angle_3
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

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu '
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10

  dt = 0.1
  dtmin = 0.01
  end_time = 0.5
[]

[Outputs]
  csv = true
[]
