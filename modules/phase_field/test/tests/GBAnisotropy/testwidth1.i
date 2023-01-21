[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 2
  nz = 0
  xmin = -500
  xmax = 1000
  ymin = 0
  ymax = 100
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 3
  var_name_base = gr
  length_scale = 1.0e-9
  time_scale = 1.0e-9
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[Functions]
  [./ic_func_eta1]
    type = ParsedFunction
    expression = '0.5*(1.0-tanh((x)*sqrt(m/kappa/2.0)))'
    symbol_names = 'm kappa'
    symbol_values = '0.26514 331.414'
  [../]
  [./ic_func_eta2]
    type = ParsedFunction
    expression = '0.5*(1.0+tanh((x)*sqrt(m/kappa/2.0)))*0.5*(1.0-tanh((x-500)*sqrt(m/kappa/2.0)))'
    symbol_names = 'm kappa'
    symbol_values = '0.26514 331.414'
  [../]
  [./ic_func_eta3]
    type = ParsedFunction
    expression = '0.5*(1.0+tanh((x-500)*sqrt(m/kappa/2.0)))'
    symbol_names = 'm kappa'
    symbol_values = '0.26514 331.414'
  [../]
[]

[ICs]
  [./eta1_ic]
    variable = gr0
    type = FunctionIC
    function = ic_func_eta1
  [../]
  [./eta2_ic]
    variable = gr1
    type = FunctionIC
    function = ic_func_eta2
  [../]
  [./eta3_ic]
    variable = gr2
    type = FunctionIC
    function = ic_func_eta3
  [../]
[]

[AuxVariables]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [./unique_grains]
    order = FIRST
    family = LAGRANGE
  [../]
  [./var_indices]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./PolycrystalKernel]
  [../]
[]

[AuxKernels]
  [./bnds_aux]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./CuGrGranisotropic]
    type = GBWidthAnisotropy
    kappa = 331.414
    mu = 0.26514
    T = 600 # K

    # molar_volume_value = 7.11e-6 #Units:m^3/mol
    Anisotropic_GB_file_name = anisotropy_energy.txt
    inclination_anisotropy = false # true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 30
  l_tol = 1e-4
  nl_max_its = 40
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-11

  num_steps = 2

  dt = 10
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
