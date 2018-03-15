#
# Testing nodal aux variables that are computed only at the end of the time step
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 3
  ny = 3
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  active = 'aux1 aux2'

  [./aux1]
    order = FIRST
    family = LAGRANGE
  [../]

  [./aux2]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'ie diff force'

  [./ie]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  #Coupling of nonlinear to Aux
  [./force]
    type = CoupledForce
    variable = u
    v = aux2
  [../]
[]

[AuxKernels]
  active = 'constant field'

  #Simple Aux Kernel
  [./constant]
    variable = aux1
    type = ConstantAux
    value = 1
  [../]

  #Shows coupling of Aux to nonlinear
  [./field]
    variable = aux2
    type = CoupledAux
    value = 2
    coupled = u
    execute_on = timestep_end
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]
[]

[Executioner]
  type = Transient

  start_time = 0
  dt = 0.1
  num_steps = 2


  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out_ts
  exodus = true
[]
