[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  second_order = true
[]

[Variables]
  [u]
    order = SECOND
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [top]
    type = FunctionDirichletBC
    variable = u
    function = 'sin(x*2*pi)'
    boundary = top
  []
[]

[AuxVariables]
  [aux]
    family = LAGRANGE
    order = SECOND
  []
[]

[AuxKernels]
  [coupled]
    type = CoupledAux
    variable = aux
    coupled = u
  []
[]

[Postprocessors]
  [max]
    type = SideExtremeValue
    variable = aux
    boundary = top
  []
  [min]
    type = SideExtremeValue
    variable = aux
    boundary = top
    value_type = min
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]
