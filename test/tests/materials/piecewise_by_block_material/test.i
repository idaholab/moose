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

[Variables]
  [dummy]
    type = MooseVariableFVReal
  []
[]

# This is added to have sufficient ghosting layers, see #19534
[FVKernels]
  [diff]
    type = FVDiffusion
    variable = 'dummy'
    coeff = 1
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
    type = ADNonFunctorSideDiffusiveFluxIntegral
    boundary = left
    variable = v
    diffusivity = 'coeff'
  []

  # to trigger ghost evaluations
  [flux_mid]
    type = ADInterfaceDiffusiveFluxIntegral
    boundary = middle
    variable = v
    diffusivity = 'coeff'
    coeff_interp_method = average
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
  # To get level of ghosting
  [console]
    type = Console
    system_info = 'framework mesh aux nonlinear execution relationship'
  []
[]
