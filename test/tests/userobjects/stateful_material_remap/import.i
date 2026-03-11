# This input file imports stateful material property data exported by export.i
# onto a different (finer) mesh using closest-point matching, then runs a
# single timestep to verify the remapped values are used.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[Variables]
  [u]
    initial_condition = 0
  []
[]

[Kernels]
  [diff]
    type = MatDiffusionTest
    variable = u
    prop_name = diffusivity
    prop_state = 'old'
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Materials]
  [stateful]
    type = SpatialStatefulMaterial
    initial_diffusivity = 1.0
  []
[]

[AuxVariables]
  [diffusivity_aux]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [diff_aux]
    type = MaterialRealAux
    variable = diffusivity_aux
    property = diffusivity
    execute_on = 'initial timestep_end'
  []
[]

[Postprocessors]
  [avg_diffusivity]
    type = ElementAverageValue
    variable = diffusivity_aux
    execute_on = 'initial timestep_end'
  []
[]

[UserObjects]
  [importer]
    type = StatefulMaterialPropertyImporter
    file = 'stateful_export.smatprop'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 1
  dt = 0.1
[]

[Outputs]
  exodus = true
  csv = true
[]
