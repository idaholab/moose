[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 10
    ny = 10
  []
[]
[Problem]
  solve = false
  kernel_coverage_check = false
[]

[AuxVariables]
  [non_local_material]
    family = MONOMIAL
  []
[]

[AuxKernels]
  [non_local]
    type = RadialAverageAux
    average_UO = ele_avg
    variable = non_local_material
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Functions]
  [func]
    type = ParsedFunction
    value = 'if(x >=0,(1+t),-(1+t))'
  []
[]
[Materials]
  [local_material]
    type = GenericFunctionMaterial
    prop_names = local
    prop_values = func
    outputs = exodus
  []
[]

[UserObjects]
  [ele_avg]
    type = RadialAverage
    material_name = local
    execute_on = "INITIAL timestep_end"
    block = 0
    radius = 0.3
  []
[]

[Executioner]
  type = Transient
  end_time = 3
  dt = 1
[]

[Outputs]
  exodus = true
[]
