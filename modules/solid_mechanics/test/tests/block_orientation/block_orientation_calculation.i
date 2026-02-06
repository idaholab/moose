[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 1
    ymax = 2
    zmax = 2
    nx = 2
    ny = 4
    nz = 4
    elem_type = HEX8
  []
  [subdomain]
    input = box
    type = SubdomainPerElementGenerator
    element_ids = '0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31'
    subdomain_ids = '0 0 0 0 1 1 1 1 2 2 2 2 3 3 3 3 4 4 4 4 5 5 5 5 6 6 6 6 7 7 7 7'
  []
[]

[AuxVariables]
  [euler_angle_1]
    order = FIRST
    family = LAGRANGE
  []
  [euler_angle_2]
    order = FIRST
    family = LAGRANGE
  []
  [euler_angle_3]
    order = FIRST
    family = LAGRANGE
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
    function = '10*t+x'
  []
  [euler_angle_2]
    type = FunctionAux
    variable = euler_angle_2
    function = '20*t+2*y'
  []
  [euler_angle_3]
    type = FunctionAux
    variable = euler_angle_3
    function = '30*t+3*z'
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
  [block_orientation]
    type = ComputeBlockOrientationByRotation
    # type = ComputeBlockOrientationByMisorientation <- can use this UO as well
    execute_on = 'TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [block_ea]
    type = BlockOrientationFromUserObject
    block_orientation_uo = block_orientation
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

  dt = 0.05
  dtmin = 0.01
  end_time = 0.2
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
