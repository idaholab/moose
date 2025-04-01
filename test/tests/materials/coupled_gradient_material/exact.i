AD = ''

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [u]
  []
  [u_gradient_x]
    family = MONOMIAL
  []
  [u_gradient_y]
    family = MONOMIAL
  []
  [u_gradient_z]
    family = MONOMIAL
  []
[]

[AuxKernels]
  [u_aux]
    type = ParsedAux
    variable = u
    expression = 'x + y * 2 + z * 3'
    use_xyzt = true
  []
  [u_gradient_x]
    type = ${AD}MaterialRealVectorValueAux
    variable = u_gradient_x
    property = gradient
    component = 0
  []
  [u_gradient_y]
    type = ${AD}MaterialRealVectorValueAux
    variable = u_gradient_y
    property = gradient
    component = 1
  []
  [u_gradient_z]
    type = ${AD}MaterialRealVectorValueAux
    variable = u_gradient_z
    property = gradient
    component = 2
  []
[]

[Materials]
  [mat]
    type = ${AD}CoupledGradientMaterial
    coupled_variable = u
    gradient_material_name = gradient
    outputs = all
    scalar_property_factor = 0.5
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
  [u_gradient_x_avg]
    type = ElementAverageValue
    variable = u_gradient_x
  []
  [u_gradient_y_avg]
    type = ElementAverageValue
    variable = u_gradient_y
  []
  [u_gradient_z_avg]
    type = ElementAverageValue
    variable = u_gradient_z
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
