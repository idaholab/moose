# Four elements, with u below y = 0 and v above it:
#
#                 v = 0
#          +--------+--------+
#          |   v    |   v    |
# y = 0    +--------+========+
#          |   u    |   u    |
#          +--------+--------+
#                Robin BC
#
# The whole line at y = 0 is crack; only its right half (===) is also new.
# Both the kernel and the user object select new by name, independently of ID.

# A moderate penalty gives a nonzero interface jump without a stiff solve.
interface_penalty = 10
# Use the residual tolerance of measure-conservation-interface.i for both checks.
residual_tolerance = 1e-10

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
  []
  [upper_block]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '-1 0 0'
    top_right = '1 1 0'
    block_id = 1
  []
  [split]
    type = SideSetsBetweenSubdomainsGenerator
    input = upper_block
    primary_block = 0
    paired_block = 1
    new_boundary = crack
  []
  [ligament]
    type = ParsedGenerateSideset
    input = split
    included_boundaries = crack
    combinatorial_geometry = 'x > 0'
    new_sideset_name = new
  []
  [renumber]
    type = RenameBoundaryGenerator
    input = ligament
    old_boundary = 'crack new'
    # The second test swaps these IDs; the boundary names and geometry stay fixed.
    new_boundary = '10 20'
  []
[]

[Variables]
  [u]
    block = 0
  []
  [v]
    block = 1
  []
[]

[UserObjects]
  [conservation]
    type = InterfaceDomainUserObjectCallCount
    block = 0
    u = u
    v = v
    robin_boundaries = bottom
    interface_boundaries = new
    interface_penalty = ${interface_penalty}
    nl_abs_tol = ${residual_tolerance}
    expected_interface_calls = 1
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
    block = 0
  []
  [force_u]
    type = BodyForce
    variable = u
    block = 0
  []
  [diff_v]
    type = CoeffParamDiffusion
    variable = v
    block = 1
    # Different diffusivities on the two blocks avoid a symmetric solution.
    D = 2
  []
[]

[InterfaceKernels]
  [penalty_interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = new
    penalty = ${interface_penalty}
  []
[]

[BCs]
  [bottom]
    type = RobinBC
    variable = u
    boundary = bottom
    coefficient = 2
  []
  [top]
    type = DirichletBC
    variable = v
    boundary = top
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 0
  nl_abs_tol = ${residual_tolerance}
[]

[Outputs]
  exodus = true
[]
