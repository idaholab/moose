#
# This test demonstrates an InterfaceKernel (InterfaceDiffusionFlux) that can
# replace a pair of integrated DiffusionFluxBC boundary conditions.
#

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    ymax = 0.5
  []
  [./box1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.51 1 0'
    input = gen
  [../]
  [./box2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.49 0 0'
    top_right = '1 1 0'
    input = box1
  [../]
  [./iface_u]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    new_boundary = 10
    input = box2
  [../]
[]

[Variables]
  [./u2]
    block = 1
    [./InitialCondition]
      type = FunctionIC
      function = 'r:=sqrt((x-0.4)^2+(y-0.5)^2);if(r<0.05,5,1)'
    [../]
  [../]
  [./v2]
    block = 2
    [./InitialCondition]
      type = FunctionIC
      function = 'r:=sqrt((x-0.7)^2+(y-0.5)^2);if(r<0.05,5,1)'
    [../]
  [../]
[]

[Kernels]
  [./u2_diff]
    type = Diffusion
    variable = u2
    block = 1
  [../]
  [./u2_dt]
    type = TimeDerivative
    variable = u2
    block = 1
  [../]
  [./v2_diff]
    type = Diffusion
    variable = v2
    block = 2
  [../]
  [./v2_dt]
    type = TimeDerivative
    variable = v2
    block = 2
  [../]
[]

[InterfaceKernels]
  [./iface]
    type = InterfaceDiffusionBoundaryTerm
    boundary = 10
    variable = u2
    neighbor_var = v2
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.002
  num_steps = 6
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
