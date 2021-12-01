[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [middle]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    new_boundary = middle
    paired_block = 1
    primary_block = 0
  []
[]

[AuxVariables]
  [u]
    type = MooseVariableFVReal
  []
  [v]
    type = MooseVariableFVReal
    [InitialCondition]
      type = FunctionIC
      function = '4 * (x - 7) * (x - 8)'
    []
  []
[]

[AuxKernels]
  # to trigger off-boundary element computations
  [to_var]
    type = ADMaterialRealAux
    variable = 'u'
    property = coeff
  []
[]

[Materials]
  [coeff_mat]
    type = ADPiecewiseConstantByBlockMaterial
    prop_name = 'coeff'
    subdomain_to_prop_value = '0 4
                               1 2'
  []
[]

[Postprocessors]
  # to trigger on boundary element computations
  [flux]
    type = ADSideDiffusiveFluxIntegral
    boundary = left
    variable = v
    diffusivity = 'coeff'
  []

  # to trigger ghost evaluations
  [flux_mid]
    type = ADSideDiffusiveFluxIntegral
    boundary = middle
    variable = v
    diffusivity = 'coeff'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
