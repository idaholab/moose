# This example uses rectangular fins
width = 1.0
thickness = 0.001
perimeter = ${fparse 2 * width + 2 * thickness}
area = ${fparse width * thickness}

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[FunctorMaterials]
  [fin_fmat]
    type = FinEfficiencyFunctorMaterial
    fin_height = 0.01
    fin_perimeter_area_ratio = ${fparse perimeter / area}
    heat_transfer_coefficient = 25.0
    thermal_conductivity = 15.0
    fin_efficiency_name = efficiency
  []
[]

[Postprocessors]
  [fin_efficiency]
    type = ElementExtremeFunctorValue
    functor = efficiency
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
