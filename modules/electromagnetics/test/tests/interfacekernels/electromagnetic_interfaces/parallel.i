# Verification Test of ParallelElectricFieldInterface
# with default materials
#
# Imposes u_parallel = v_parallel on each interface
# between subdomain 0 and 1

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 10
    xmax = 2
    ymax = 2
    zmax = 2
    elem_type = HEX20
  []
  [subdomain1]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 1 1'
    block_id = 1
    input = gmg
  []
  [break_boundary]
    type = BreakBoundaryOnSubdomainGenerator
    input = subdomain1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = break_boundary
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
[]


[Variables]
  [u]
    order = FIRST
    family = NEDELEC_ONE
    block = 0
  []
  [v]
    order = FIRST
    family = NEDELEC_ONE
    block = 1
  []
[]

[Kernels]
  [curl_u]
    type = CurlCurlField
    variable = u
    block = 0
  []
  [coeff_u]
    type = VectorFunctionReaction
    variable = u
    block = 0
  []
  [ffn_u]
    type = VectorBodyForce
    variable = u
    block = 0
    function_x = 1
    function_y = 1
    function_z = 1
  []
  [curl_v]
    type = CurlCurlField
    variable = v
    block = 1
  []
  [coeff_v]
    type = VectorFunctionReaction
    variable = v
    block = 1
  []
[]

[InterfaceKernels]
  [parallel]
    type = ParallelElectricFieldInterface
    variable = u
    neighbor_var = v
    boundary = primary0_interface
  []
[]

[BCs]
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]
