# This is testing a scenario where volumetric system (like kernels) asks for second derivatives
# and the formulation includes a system using neighbor elements (like DGKernels or
# InterfaceKernels)
# If the latter did not request the second derivatives MOOSE should not be computing those.
# The PDEs solved are quite contrived, the Biharmonic kernel is there just to trigger the
# computation of second derivatives.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmax = 2
    ymax = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 2 0'
    block_id = 1
  [../]
  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'middle'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./bh]
    type = Biharmonic
    variable = u
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = InterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = middle
    D = 4
    D_neighbor = 2
  [../]
[]

[BCs]
  [./u]
    type = DirichletBC
    variable = u
    value = 1
    boundary = 'right middle'
  [../]
  [./v]
    type = DirichletBC
    variable = v
    value = 2
    boundary = 'left middle'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
