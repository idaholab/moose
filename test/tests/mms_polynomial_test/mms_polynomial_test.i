#MMS.i
#This is for u = a*x^3*y*t+b*y^2*z+e*x*y*z^4
[Mesh]
 [./Generation] #We are generating our own Mesh
   dim = 3
   nx = 3
   ny = 3
   nz = 3
   x min = 0
   x max = 1
   y min = 0
   y max = 1
   z min = 0
   z max = 1
   elem_type = HEX8
  [../]
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
    type = ImplicitEuler
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

[BCs]
  active = 'all_u'

  [./all_u]
    type = PolyCoupledDirichletBC
    variable = u
    boundary = '0 1 2 3 4 5'
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 0 #This is 0 because we are creating our own Mesh
  [../]
[]

[Executioner]
  type = Transient
  dt = .1
  num_steps = 20
  petsc_options = "-snes_mf_operator"
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  output_initial = true
  perf_log = true
[]


