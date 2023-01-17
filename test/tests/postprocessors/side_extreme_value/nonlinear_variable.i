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

[Postprocessors]
  [max]
    type = SideExtremeValue
    variable = u
    boundary = top
  []
  [min]
    type = SideExtremeValue
    variable = u
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
