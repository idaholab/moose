#
# Test a gradient continuity interfacekernel
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 10
  ymax = 0.5
[]

[MeshModifiers]
  [./box1]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.51 1 0'
  [../]
  [./box2]
    type = SubdomainBoundingBox
    block_id = 2
    bottom_left = '0.49 0 0'
    top_right = '1 1 0'
  [../]
  [./iface_u]
    type = SideSetsBetweenSubdomains
    master_block = 1
    paired_block = 2
    new_boundary = 10
    depends_on = 'box1 box2'
  [../]
  [./iface_v]
    type = SideSetsBetweenSubdomains
    master_block = 2
    paired_block = 1
    new_boundary = 11
    depends_on = 'box1 box2'
  [../]
[]

[Variables]
  [./u]
    block = 1
    [./InitialCondition]
      type = FunctionIC
      function = 'r:=sqrt((x-0.4)^2+(y-0.5)^2);if(r<0.05,5,1)'
    [../]
  [../]
  [./v]
    block = 2
    initial_condition = 0.8
  [../]
[]

[Kernels]
  [./u_diff]
    type = Diffusion
    variable = u
    block = 1
  [../]
  [./u_dt]
    type = TimeDerivative
    variable = u
    block = 1
  [../]
  [./v_diff]
    type = Diffusion
    variable = v
    block = 2
  [../]
  [./v_dt]
    type = TimeDerivative
    variable = v
    block = 2
  [../]
[]

[InterfaceKernels]
  [./iface]
    type = InterfaceDiffusionFluxMatch
    variable = u
    boundary = 10
    neighbor_var = v
  [../]
[]

[BCs]
  [./u_boundary_term]
    type = DiffusionFluxBC
    variable = u
    boundary = 10
  [../]
  [./v_boundary_term]
    type = DiffusionFluxBC
    variable = v
    boundary = 11
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.002
  num_steps = 10
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
