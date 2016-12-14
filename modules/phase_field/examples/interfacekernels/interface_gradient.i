[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
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
  [./iface]
    type = SideSetsBetweenSubdomains
    master_block = 1
    paired_block = 2
    new_boundary = 10
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
  [./gradient_continuity]
    type = InterfaceGradientMatch
    variable = u
    boundary = 10
    neighbor_var = v
    component = 0
  [../]
  [./diffusion_surface_term]
    type = InterfaceDiffusionFlux
    boundary = 10
    variable = u
    neighbor_var = v
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.001
  num_steps = 20
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
