# Validates KokkosNEML2ToMOOSEMaterialProperty reading a NEML2 tangent, which is the case where the
# output does not vary along the batch.
#
# NEML2 returns a derivative that is constant across the batch as a broadcast: one set of components
# with a batch stride of zero, produced by dynamic_expand(). Compacting that to alias it would duplicate
# the components once per quadrature point, so the material aliases the single set instead, and storing
# the property with constant_on = SUBDOMAIN keeps one copy per subdomain rather than one per quadrature
# point. For a linear elastic model the tangent is the elasticity tensor, so it is genuinely constant.
#
# The non-Kokkos bridge reads the same derivative per quadrature point and the two are differenced, so
# the expected result is zero rather than a stored gold value. The magnitude is reported as well, so
# that a difference of zero cannot be mistaken for both paths producing nothing.
#
# The displacement field is prescribed through auxiliary variables rather than solved, because only the
# tangent retrieval is under test.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
  []
[]

# A trivial nonlinear problem, so that the executioner has something to advance
[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    # Ramped so that each step has a genuine solve; a static value leaves the problem converged
    # after the first step and the line search then reports failure with nothing left to do
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = 't'
  []
[]

[Functions]
  [ramp_x]
    type = ParsedFunction
    expression = '0.002 * t * x'
  []
  [ramp_y]
    type = ParsedFunction
    expression = '-0.001 * t * y + 0.0005 * t * x'
  []
  [ramp_z]
    type = ParsedFunction
    expression = '0.0003 * t * z'
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [host_s]
    order = CONSTANT
    family = MONOMIAL
  []
  [device_s]
    order = CONSTANT
    family = MONOMIAL
  []
  [difference]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [set_disp_x]
    type = FunctionAux
    variable = disp_x
    function = ramp_x
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [set_disp_y]
    type = FunctionAux
    variable = disp_y
    function = ramp_y
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [set_disp_z]
    type = FunctionAux
    variable = disp_z
    function = ramp_z
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  # Reference: the non-Kokkos bridge's host tangent, stored per quadrature point
  [host_s]
    type = MaterialSymmetricRankFourTensorAux
    variable = host_s
    property = 'dneml2_stress/dneml2_strain'
    i = 0
    j = 1
  []
  # The Kokkos bridge's device tangent, stored once per subdomain
  [device_s]
    type = KokkosSymmetricRankFourComponentAux
    variable = device_s
    property = 'kokkos_neml2_jacobian'
    i = 0
    j = 1
  []
  [difference]
    type = ParsedAux
    variable = difference
    coupled_variables = 'host_s device_s'
    expression = 'device_s - host_s'
  []
[]

[NEML2]
  input = '../../neml2/elasticity/elasticity_neml2.i'
  [all]
    model = 'model'
    device = 'cuda'
    output_device = 'cuda'
    derivatives = 'neml2_stress neml2_strain'
  []
[]

[Materials]
  [strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  []
  [convert_strain]
    type = RankTwoTensorToSymmetricRankTwoTensor
    from = 'mechanical_strain'
    to = 'neml2_strain'
  []
  # The Kokkos bridge, reading the same tangent on the device. constant_on = SUBDOMAIN stores it once
  # per subdomain, which the material accepts only because the output is a broadcast.
  [kokkos_jacobian]
    type = KokkosNEML2ToMOOSESymmetricRankFourTensorMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'neml2_stress'
    neml2_input_derivative = 'neml2_strain'
    to_moose = 'kokkos_neml2_jacobian'
    constant_on = SUBDOMAIN
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 2
  dt = 1.0
[]

[Postprocessors]
  # Zero when the device bridge reproduces the host bridge
  [max_abs_difference]
    type = ElementExtremeValue
    variable = difference
    value_type = max_abs
  []
  # Reported so that a difference of zero cannot be mistaken for both paths producing nothing
  [reference_magnitude]
    type = ElementExtremeValue
    variable = host_s
    value_type = max_abs
  []
[]

[Outputs]
  csv = true
[]
