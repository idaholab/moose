[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 250
  ymax = 250
  elem_type = QUAD4
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = RandomIC
    [../]
  [../]
[]

[Kernels]
  [./w_res]
    type = Diffusion
    variable = c
  [../]
  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[Materials]
  [./free_energy1]
    type = DerivativeParsedMaterial
    f_name = Fa
    args = 'c'
    function = (c-0.1)^4*(1-0.1-c)^4
  [../]
  [./free_energy2]
    type = DerivativeParsedMaterial
    f_name = Fb
    args = 'c'
    function = -0.25*(c-0.1)^4*(1-0.1-c)^4
  [../]

  # Fa+Fb+Fb == Fc
  [./free_energy3]
    type = DerivativeParsedMaterial
    f_name = Fc
    args = 'c'
    function = 0.5*(c-0.1)^4*(1-0.1-c)^4
    outputs = all
  [../]
  [./dfree_energy3]
    type = DerivativeParsedMaterial
    f_name = dFc
    args = 'c'
    material_property_names = 'F:=D[Fc,c]'
    function = F
    outputs = all
  [../]
  [./d2free_energy3]
    type = DerivativeParsedMaterial
    f_name = d2Fc
    args = 'c'
    material_property_names = 'F:=D[Fc,c,c]'
    function = F
    outputs = all
  [../]

  [./free_energy]
    type = DerivativeSumMaterial
    f_name = F_sum
    sum_materials = 'Fa Fb Fb'
    args = 'c'
    outputs = all
  [../]
  [./dfree_energy]
    type = DerivativeParsedMaterial
    f_name = dF_sum
    material_property_names = 'F:=D[F_sum,c]'
    function = F
    args = 'c'
    outputs = all
  [../]
  [./d2free_energy]
    type = DerivativeParsedMaterial
    f_name = d2F_sum
    material_property_names = 'F:=D[F_sum,c,c]'
    function = F
    args = 'c'
    outputs = all
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [./F_sum_pp]
    type = ElementAverageValue
    variable = F_sum
  [../]
  [./F_check]
    type = ElementAverageValue
    variable = Fc
  [../]
  [./dF_sum_pp]
    type = ElementAverageValue
    variable = dF_sum
  [../]
  [./dF_check]
    type = ElementAverageValue
    variable = dFc
  [../]
  [./d2F_sum_pp]
    type = ElementAverageValue
    variable = d2F_sum
  [../]
  [./d2F_check]
    type = ElementAverageValue
    variable = d2Fc
  [../]
[]

[Outputs]
  exodus = true
[]
