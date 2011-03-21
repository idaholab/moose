[Mesh]
  dim = 2
  file = square.e
  uniform_refine = 4
[]

[Variables]
  active = 'diffused'

  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

# Here is the AuxVariables section where we declare the variables that
# will hold the AuxKernel calcuations.  The declaration syntax is very 
# similar to that of the regular variables section
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
    variable = diffused
  [../]
[]

# Here is the AuxKernels section where we enable the AuxKernels, link
# them to our AuxVariables, set coupling parameters, and set input parameters 
[AuxKernels]
  active = 'nodal_example element_example'
  
  [./nodal_example]
    type = ExampleAux
    variable = nodal_aux
    value = 3.0
    coupled = diffused
  [../]

  [./element_example]
    type = ExampleAux
    variable = element_aux
    value = 4.0
    coupled = diffused
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = diffused
    boundary = '1 3'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = diffused
    boundary = '2 4'
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
