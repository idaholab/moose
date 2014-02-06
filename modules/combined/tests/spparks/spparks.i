[GlobalParams]
  xmin = 0
  ymin = 0
  zmin = 0
  xmax = 20
  ymax = 20
  zmax = 20
[]

[Mesh]#Comment
  type = GeneratedMesh
#  dim = 3
  dim = 2
  nx = 20
  ny = 20
  nz = 20
[] # Mesh

[AuxVariables]
  [./var1]
  [../]
[]

[AuxKernels]
  [./fred]
    type = SPPARKSAux
    user_object = spparks
    ivar = 1
    variable = var1
    execute_on = timestep
  [../]
[]


[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  nl_abs_tol = 1e-10

  l_max_its = 20

  start_time = 0.0
  dt = 30.0
  num_steps = 2
  end_time = 60.0
[] # Executioner

[Output]
  interval = 1
  output_initial = true
  exodus = true
#  perf_log = true
  linear_residuals = true
[] # Output

[UserObjects]
  [./spparks]
    type = SPPARKSUserObject
#    file = in.potts
    file = in.potts2D
    from_ivar = 1
#    spparks_only = true
  [../]
[]

[Problem]
  solve = 0
[]
