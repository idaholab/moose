[Mesh]
  type = GeneratedMesh
  elem_type = HEX8
  dim = 3
  nz = 10
  xmax = 10
  ymax = 10
  zmax = 100
[]

[Variables]
  [./x_disp]
    block = 0
  [../]
  [./y_disp]
    block = 0
  [../]
  [./z_disp]
    block = 0
  [../]
[]

[TensorMechanics]
  [./solid]
#    disp_x = x_disp
#    disp_y = y_disp
#    disp_z = z_disp
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Materials]
  active = 'fcrysp'
  [./felastic]
    type = FiniteStrainElasticMaterial
    block = 0
    fill_method = symmetric9
    disp_x = x_disp
    disp_y = y_disp
    disp_z = z_disp
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
  [../]
  [./fcrysp]
    type = FiniteStrainCrystalPlasticity
    block = 0
    disp_y = y_disp
    disp_x = x_disp
    disp_z = z_disp
    flowprops = '1 12 0.001 0.1'
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    nss = 12
    hprops = '1.0 541.5 60.8 109.8'
    gprops = '1 12 60.8'
    fill_method = symmetric9
    slip_sys_file_name = input_slip_sys.txt
  [../]
[]

[Functions]
  [./topdisp]
    type = ParsedFunction
    value = 0.7*t
  [../]
  [./tpress]
    type = ParsedFunction
    value = -200*t
  [../]
[]

[BCs]
  [./zbc]
    type = DirichletBC
    variable = z_disp
    boundary = back
    value = 0
  [../]
  [./ybc]
    type = DirichletBC
    variable = y_disp
    boundary = bottom
    value = 0
  [../]
  [./xbc]
    type = DirichletBC
    variable = x_disp
    boundary = left
    value = 0
  [../]
  [./zmove]
    type = FunctionDirichletBC
    variable = z_disp
    boundary = front
    function = topdisp
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./e_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 3
    index_j = 3
    execute_on = timestep_end
    block = 0
  [../]
  [./e_zz]
    type = RankTwoAux
    rank_two_tensor = lage
    variable = e_zz
    index_i = 3
    index_j = 3
    execute_on = timestep_end
    block = 0
  [../]
[]

[Postprocessors]
  [./szz]
    type = ElementAverageValue
    variable = stress_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./ezz]
    type = ElementAverageValue
    variable = e_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  num_steps = 1000
  end_time = 1
  dt = 0.02
  dtmax = 0.02
  dtmin = 0.02
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomerang
  nl_abs_tol = 1e-08
  nl_rel_step_tol = 1e-08
  nl_abs_step_tol = 1e-08
  abort_on_solve_fail = true
  n_startup_steps = 0.0
[]

[Outputs]
  file_base = out
  exodus = true
  csv = true
[]
