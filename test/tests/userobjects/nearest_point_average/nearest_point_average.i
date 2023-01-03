[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 8
  ny = 8
  nz = 8
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v]
  []
  [np_average]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [v]
    type = FunctionIC
    variable = v
    function = v
  []
[]

[Functions]
  [v]
    type = ParsedFunction
    value = x+y-sin(z)
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [np_average]
    type = SpatialUserObjectAux
    variable = np_average
    execute_on = timestep_end
    user_object = npa
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
[]

[UserObjects]
  [npa]
    type = NearestPointAverage
    points_file = points.txt
    variable = v
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  hide = 'u'
[]
