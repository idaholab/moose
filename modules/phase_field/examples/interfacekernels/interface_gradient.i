[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
  []
  [./box1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.51 1 0'
  [../]
  [./box2]
    input = box1
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.49 0 0'
    top_right = '1 1 0'
  [../]
  [./iface]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    new_boundary = 10
    input = box2
  [../]
  [./rotate]
    type = TransformGenerator
    transform = ROTATE
    vector_value = '5 0 0'
    input = iface
  [../]
[]

[GlobalParams]
  order = FIRST
  family = LAGRANGE
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
  [./flux_continuity]
    type = InterfaceDiffusionFluxMatch
    variable = u
    boundary = 10
    neighbor_var = v
  [../]
  [./diffusion_surface_term]
    type = InterfaceDiffusionBoundaryTerm
    boundary = 10
    variable = u
    neighbor_var = v
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'

  dt = 0.001
  num_steps = 20
[]

[Outputs]
  [./out]
    type = Exodus
    use_problem_dimension = false
  [../]
  print_linear_residuals = false
[]
