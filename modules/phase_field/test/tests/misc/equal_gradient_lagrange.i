#
# This test demonstrates an InterfaceKernel set that can enforce the componentwise
# continuity of the gradient of a variable using the Lagrange multiplier method.
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
    initial_condition = 0.8
  [../]
  [./lambda]
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
  [./lambda]
    type = NullKernel
    variable = lambda
  [../]
[]

[InterfaceKernels]
  [./iface]
    type = InterfaceDiffusionBoundaryTerm
    boundary = 10
    variable = u2
    neighbor_var = v2
  [../]
  [./lambda]
    type = EqualGradientLagrangeMultiplier
    variable = lambda
    boundary = 10
    element_var = u2
    neighbor_var = v2
    component = 0
  [../]
  [./constraint]
    type = EqualGradientLagrangeInterface
    boundary = 10
    lambda = lambda
    variable = u2
    neighbor_var = v2
    component = 0
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[VectorPostprocessors]
  [./uv]
    type = LineValueSampler
    variable = 'u2 v2'
    start_point = '0 0.5 0'
    end_point = '1 0.5 0'
    sort_by = x
    num_points = 100
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pctype -sub_pc_type -sub_pc_factor_shift_type -pc_factor_shift_type'
  petsc_options_value = ' asm    lu          nonzero                    nonzero'

  dt = 0.002
  num_steps = 10
[]

[Outputs]
  exodus = true
  csv = true
  hide = lambda
  print_linear_residuals = false
[]
