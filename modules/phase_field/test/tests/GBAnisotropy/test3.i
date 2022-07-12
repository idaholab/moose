[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 1000
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  op_num = 2
  var_name_base = gr
  wGB = 100
  length_scale = 1.0e-9
  time_scale = 1.0e-9
[]

[Variables]
  [./PolycrystalVariables]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./BicrystalCircleGrainIC]
      radius = 333.33
      x = 500
      y = 500
    [../]
  [../]
[]

[AuxVariables]
  [./bnds]
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

[BCs]
  [./Periodic]
    [./top_bottom]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./CuGrGranisotropic]
    type = GBAnisotropy
    T = 600 # K

    # molar_volume_value = 7.11e-6 #Units:m^3/mol
    Anisotropic_GB_file_name = anisotropy.txt

    inclination_anisotropy = true
    delta_sigma = 0.1
    delta_mob = 0.0
  [../]
[]

[Postprocessors]
  [./dt]
    # Outputs the current time step
    type = TimestepSize
  [../]

  [./gr1_area]
    type = ElementIntegralVariablePostprocessor
    variable = gr1
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 30
  l_tol = 1e-4
  nl_max_its = 40
  nl_rel_tol = 1e-9

  num_steps = 1
  dt = 150.0
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
