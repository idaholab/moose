[Mesh/gen]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables/aux]
  family = MONOMIAL
[]

[AuxKernels/mat]
  type = MaterialRealAux
  variable = aux
  property = prop
[]

[Problem]
  solve = false
  material_dependency_check = false
[]

[Materials]
  [prop0]
    type = GenericConstantMaterial
    prop_names = "prop"
    prop_values = "1"
  []
  [prop1]
    type = GenericConstantMaterial
    prop_names = "prop"
    prop_values = "2"
    enable = false
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
[]

[Outputs]
  csv = true
[]

[Postprocessors/avg]
  type = ElementAverageValue
  variable = aux
[]

[Controls/mat_control]
  type = TimePeriod
  enable_objects = '*/prop1'
  disable_objects = '*/prop0'
  start_time = 0.5
  end_time = 1
  execute_on = 'INITIAL TIMESTEP_END'
[]
