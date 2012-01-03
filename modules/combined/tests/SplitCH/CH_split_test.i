[Mesh]
  [./Generation]
    dim = 2
    nx = 10
    ny = 10
    nz = 0
    xmin = 0
    xmax = 50
    ymin = 0
    ymax = 50
    zmin = 0
    zmax = 50
    elem_type = QUAD4
  [../]
  uniform_refine = 1
[]

[Variables]

  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 25.0
      y1 = 25.0
      radius = 10.0
      invalue = 1.0
      outvalue = -0.9
      int_width = 6.0
    [../]
  [../]

  [./w]
    order = FIRST 
    family = LAGRANGE
  
  [../]
[]	     

[Kernels]

  [./split2chempot]
    type = CHSplit2ChemPot
    variable = c
    w = w
    kappa_name = kappa_c
  [../]

  [./time]
    type = TimeDerivative
    variable = c
  [../]

  [./split1]
    type = CHSplit1
    variable = w
    mob_name = M
    c = c
  [../]
[]

[BCs]
active = 'Periodic'
  [./Periodic]
    [./left_right]
      primary = 0
      secondary = 2
      translation = '0 50 0'
    [../]

    [./top_bottom]
      primary = 1
      secondary = 3
      translation = '-50 0 0'
    [../]
  [../]
[]

[Materials]

  [./constant]
    type = PFMobility
    block = 0
    mob = 1.0
    kappa = 1.0
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'crank-nicolson'
  petsc_options = '-snes_mf_operator -ksp_monitor'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'

  l_max_its = 15
  nl_max_its = 10

  start_time = 0.0
  num_steps = 2
  dt = 0.1
 
  [./Adaptivity]
    initial_adaptivity = 1
    refine_fraction = 0.2
   coarsen_fraction = 0.25
    max_h_level = 2
  [../]
[]

[Output]
  file_base = circle
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
   
    
