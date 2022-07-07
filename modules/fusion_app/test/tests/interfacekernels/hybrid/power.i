[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    xmax = 2
    ny = 40
    ymax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.8 0.8 0'
    top_right = '1.2 1.2 0'
    block_id = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary1_interface'
  []
  [blockdeletion]
    input = interface
    type = BlockDeletionGenerator
    block = 1
  []
[]

[Variables]
  [temp][]
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = temp
    diffusivity = D
    block = 0
  []
  [heat_generation]
    type = CoupledForce
    variable = temp
    v = power_density
  []
[]

[AuxVariables]
  [power_density][]
[]

[AuxKernels]
  [constant]
    type = ConstantAux
    value = 1
    variable = power_density
  []
[]

[Postprocessors]
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = power_density
  []
  [flux_inner]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 'primary1_interface'
    diffusivity = 'D'
  []
  [flux_outer]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 'top bottom left right'
    diffusivity = 'D'
  []
  [flux]
    type = LinearCombinationPostprocessor
    pp_names = "flux_inner flux_outer"
    pp_coefs = "1 1"
  []
[]

[BCs]
  [interface_bc]
    type = DirichletBC
    variable = temp
    value = 0.1
    boundary = primary1_interface
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[Materials]
  [mat0]
    type = GenericConstantMaterial
    prop_names = 'D'
    prop_values = '1'
  []
[]
