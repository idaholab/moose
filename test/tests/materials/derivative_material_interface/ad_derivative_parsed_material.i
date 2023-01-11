#
# Test the AD version of derivative parsed material
#

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = ADMatDiffusion
    variable = eta
    diffusivity = F
  []
  [./dt]
    type = TimeDerivative
    variable = eta
  []
[]

[Materials]
  [./Fbar]
    type = ADDerivativeParsedMaterial
    coupled_variables  = 'eta'
    property_name = Fbar
    expression ='1/3*(eta-0.5)^3'
  []
  [./F]
    type = ADParsedMaterial
    coupled_variables  = 'eta'
    material_property_names = 'F:=D[Fbar,eta]'
    expression ='F'
    outputs = exodus
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.1
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
