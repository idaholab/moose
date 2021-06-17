[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./fp_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./gss1]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./tdisp]
    type = ParsedFunction
    value = 0.01*t
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  [../]
  [./fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = fp
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  [../]
  [./e_zz]
    type = RankTwoAux
    variable = e_zz
    rank_two_tensor = lage
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  [../]
  [./gss1]
    type = MaterialStdVectorAux
    variable = gss1
    property = gss
    index = 0
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./tdisp]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = tdisp
  [../]
[]

[Materials]
  [./crysp]
    type = FiniteStrainCPSlipRateRes
    gtol = 1e-2
    rtol = 1e-8
    abs_tol = 1e-15
    slip_sys_file_name = input_slip_sys.txt
    nss = 12
    num_slip_sys_flowrate_props = 2 #Number of properties in a slip system
    flowprops = '1 4 0.001 0.1 5 8 0.001 0.1 9 12 0.001 0.1'
    hprops = '1.0 541.5 60.8 109.8 2.5'
    gprops = '1 4 60.8 5 8 60.8 9 12 60.8'
    tan_mod_type = exact
    slip_incr_tol = 1
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Postprocessors]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  [../]
  [./e_zz]
    type = ElementAverageValue
    variable = e_zz
  [../]
  [./gss1]
    type = ElementAverageValue
    variable = gss1
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  dt = 0.2
  dtmin = 0.05
  dtmax = 10.0

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  end_time = 1
[]

[Outputs]
  file_base = out
  exodus = true
  print_linear_residuals = true
  perf_graph = true
[]
