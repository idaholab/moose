[Mesh]
  dim = 2
  file = square.e
  uniform_refine = 4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  active = 'nodal_aux element_aux'

  [./nodal_aux]
    order = FIRST
    family = LAGRANGE
  [../]

  [./element_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]
  
[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  active = 'nodal_example element_example'
  
  [./nodal_example]
    type = ExampleAux
    variable = nodal_aux
    value = 3.0
    coupled_to = u
    coupled_as = coupled
  [../]

  [./element_example]
    type = ExampleAux
    variable = element_aux
    value = 4.0
    coupled_to = u
    coupled_as = coupled
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = '1 3'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = '2 4'
    value = 1
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Execution]
  type = Steady
  perf_log = true
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
