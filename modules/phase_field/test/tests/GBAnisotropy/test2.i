[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 60
  ny = 30
  nz = 0
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 600
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables] # produce smooth initial GB
  [./gr0]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SpecifiedSmoothCircleIC
      x_positions = '250.0  750.0'
      y_positions = '300.0  300.0'
      z_positions = '  0.0    0.0'
      radii       = '200.0  200.0'
      invalue = 0.0
      outvalue = 1.0
      int_width = 50.0
    [../]
  [../]
  [./gr1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 250.0
      y1 = 300.0
      radius = 200.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 50.0
    [../]
  [../]
  [./gr2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 750.0
      y1 = 300.0
      radius = 200.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 50.0
    [../]
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
  var_name_base = gr
  op_num = 3
  [../]
[]

[AuxKernels]
  [./bnds_aux]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
    var_name_base = gr
    op_num = 3
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

    op_num = 3
    var_name_base = gr
    wGB = 100
    length_scale = 1.0e-9
    time_scale = 1.0e-9

    # molar_volume_value = 7.11e-6 #Units:m^3/mol
    Anisotropic_GB_file_name = anisotropy_mobility.txt
    inclination_anisotropy = false
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
  [./gr2_area]
    type = ElementIntegralVariablePostprocessor
    variable = gr2
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

  dt = 5.0
  num_steps = 2
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
