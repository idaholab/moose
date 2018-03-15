#2_d_mms_sine_test.i
#This is for u = sin(a*x*y*z*t)
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables] #We added nodal AuxVariables
  active = 'nodal_aux'

  [./nodal_aux]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]

  active = 'diff implicit conv forcing reaction'

  [./diff]
    type = MMSDiffusion
    variable = u
  [../]

  [./implicit] #We got from MOOSE kernels
    type = MMSImplicitEuler
    variable = u
  [../]

  [./conv] #We created our own convection kernel
    type = MMSConvection
    variable = u
    x = -1
    y = 2
  [../]

  [./forcing] #We created our own forcing kernel
    type = MMSForcing
    variable = u
  [../]

  [./reaction] #We got from MOOSE kernels
    type = MMSReaction
    variable = u
  [../]
[]
[AuxKernels] #We created our own AuxKernel
  active = 'ConstantAux'

  [./ConstantAux]
    type = MMSConstantAux
    variable = nodal_aux
  [../]
[]

[BCs]
  active = 'all_u'

  [./all_u]
    type = MMSCoupledDirichletBC
    variable = u
    boundary = '0 1 2 3'
 #   value = sin(a*x*y*z*t)
  [../]
[]

[Executioner]
  type = Transient
  dt = .1
  num_steps = 5

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = 2_d_out
  exodus = true
[]
