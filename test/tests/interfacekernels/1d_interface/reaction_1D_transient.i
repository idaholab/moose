# Transient-state test for the InterfaceReaction kernel.
#
# Same to steady-state, except the following
#
# Natural BCs are applied (i.e. NewmannBC h=0 at left and right)

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  [../]
  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'subdomain1'
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = '0'
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
    block = '1'
  [../]
[]

[Kernels]
  [./diff_u]
    type = MatDiffusion
    diffusivity = D
    variable = u
    block = '0'
  [../]
  [./diff_v]
    type = MatDiffusion
    diffusivity = D
    variable = v
    block = '1'
  [../]
  [./diff_u_dt]
    type = TimeDerivative
    variable = u
    block = '0'
  [../]
  [./diff_v_dt]
    type = TimeDerivative
    variable = v
    block = '1'
  [../]
  [./source_u]
    type = BodyForce
    variable = u
    block = '0'
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = InterfaceDiffusion
    variable = u
    neighbor_var = 'v'
    boundary = 'primary0_interface'
    D = D
    D_neighbor = D
  [../]
  [./interface_reaction]
    type = InterfaceReaction
    variable = u
    neighbor_var = 'v'
    boundary = 'primary0_interface'
    kf = 1 # Forward reaction rate coefficient
    kb = 2 # Backward reaction rate coefficient
  [../]
[]

[Materials]
  [./block0]
    type = GenericConstantMaterial
    block = '0'
    prop_names = 'D'
    prop_values = '4'
  [../]
  [./block1]
    type = GenericConstantMaterial
    block = '1'
    prop_names = 'D'
    prop_values = '2'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[Debug]
  show_var_residual_norms = true
[]
