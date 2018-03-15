#MMS.i
#This is for u = a*x^3*y*t+b*y^2*z+e*x*y*z^4
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
  elem_type = HEX8
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
    type = PolyDiffusion
    variable = u
  [../]

  [./implicit] #We got from MOOSE kernels
    type = TimeDerivative
    variable = u
  [../]

  [./conv] #We created our own convection kernel
    type = PolyConvection
    variable = u
    x = -1
    y = 2
    z = -3
  [../]

  [./forcing] #We created our own forcing kernel
    type = PolyForcing
    variable = u
  [../]

  [./reaction] #We got from MOOSE kernels
    type = PolyReaction
    variable = u
  [../]
[]
[AuxKernels] #We created our own AuxKernel
  active = 'ConstantAux'

  [./ConstantAux]
    type = PolyConstantAux
    variable = nodal_aux
  [../]
[]

[BCs]
  active = 'all_u'

  [./all_u]
    type = PolyCoupledDirichletBC
    variable = u
    boundary = '0 1 2 3 4 5'
  [../]
[]

[Executioner]
  type = Transient
  dt = .1
  num_steps = 20

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  exodus = true
[]
