# Validates KokkosMOOSESymmetricRankTwoTensorToNEML2, which gathers a Kokkos material property into a
# NEML2 input variable on the device.
#
# The strain NEML2 consumes is produced by a Kokkos material from a vector displacement variable and
# handed to NEML2 by the Kokkos gatherer, replacing the host gatherer the NEML2 action would
# otherwise create. Everything downstream is unchanged from the neml2_bridge test, so the reported
# stress magnitude must reproduce that test's values: the constitutive problem is identical and only
# the route the strain takes into NEML2 differs.
#
# The model is stateful, and its old state is gathered on the device as well: each stateful NEML2
# output is retrieved into a Kokkos material property and its old value is gathered back as the
# corresponding input. Every per-quadrature-point quantity entering and leaving the model therefore
# uses one numbering, so nothing depends on the Kokkos numbering agreeing with
# NEML2BatchIndexGenerator's. The second step is what exercises the state round trip, since the old
# state is zero on the first one.
#
# The displacement field is prescribed through an auxiliary variable rather than solved, because only
# the strain gather and the constitutive update are under test.

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
  # The same prescribed displacement as the neml2_bridge test, as one vector function
  [ramp]
    type = ParsedVectorFunction
    expression_x = '0.002 * t * x'
    expression_y = '-0.001 * t * y + 0.0005 * t * x'
    expression_z = '0.0003 * t * z'
  []
[]

[AuxVariables]
  [disp]
    family = LAGRANGE_VEC
    order = FIRST
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
  [set_disp]
    type = VectorFunctionAux
    variable = disp
    function = ramp
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  # The NEML2 stress retrieved by the non-Kokkos bridge the action creates
  [host_s]
    type = MaterialSymmetricRankTwoTensorAux
    variable = host_s
    property = 'neml2_stress'
    component = 5
  []
  # The same NEML2 output retrieved on the device
  [device_s]
    type = KokkosSymmetricRankTwoComponentAux
    variable = device_s
    property = 'kokkos_neml2_stress'
    component = 5
  []
  [difference]
    type = ParsedAux
    variable = difference
    coupled_variables = 'host_s device_s'
    expression = 'device_s - host_s'
  []
[]

[NEML2]
  input = '../../neml2/plasticity/isoharden_neml2.i'
  [all]
    model = 'model'
    device = 'cuda'
    output_device = 'cuda'
    # Every per-quadrature-point input is supplied by a Kokkos gatherer below instead of a host one
    input_kernels = 'neml2_strain plastic_strain~1 equivalent_plastic_strain~1 flow_rate~1'
  []
[]

[UserObjects]
  [neml2_strain]
    type = KokkosMOOSESymmetricRankTwoTensorToNEML2
    from_moose = 'mechanical_strain'
    to_neml2 = 'neml2_strain'
  []
  # The old state, read from the Kokkos properties the corresponding outputs are retrieved into.
  # Requesting the old value is what makes those properties stateful.
  [plastic_strain~1]
    type = KokkosMOOSEOldSymmetricRankTwoTensorToNEML2
    from_moose = 'kokkos_plastic_strain'
    to_neml2 = 'plastic_strain~1'
  []
  [equivalent_plastic_strain~1]
    type = KokkosMOOSEOldRealToNEML2
    from_moose = 'kokkos_equivalent_plastic_strain'
    to_neml2 = 'equivalent_plastic_strain~1'
  []
  [flow_rate~1]
    type = KokkosMOOSEOldRealToNEML2
    from_moose = 'kokkos_flow_rate'
    to_neml2 = 'flow_rate~1'
  []
[]

[Materials]
  [strain]
    type = KokkosSymmetricComputeSmallStrain
    displacements = 'disp'
  []
  [kokkos_stress]
    type = KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'neml2_stress'
    to_moose = 'kokkos_neml2_stress'
  []
  # The stateful outputs, retrieved into Kokkos properties so the old state can be gathered on the
  # device by the user objects above
  [kokkos_plastic_strain]
    type = KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'plastic_strain'
    to_moose = 'kokkos_plastic_strain'
  []
  [kokkos_equivalent_plastic_strain]
    type = KokkosNEML2ToMOOSERealMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'equivalent_plastic_strain'
    to_moose = 'kokkos_equivalent_plastic_strain'
  []
  [kokkos_flow_rate]
    type = KokkosNEML2ToMOOSERealMaterialProperty
    neml2_executor = 'neml2_model_all'
    from_neml2 = 'flow_rate'
    to_moose = 'kokkos_flow_rate'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 2
  dt = 1.0
[]

[AuxVariables]
  [weighted_s]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # Weighting by position makes the integral below sensitive to the stress field being permuted among
  # quadrature points, which an extreme value or a plain integral is not
  [weighted_s]
    type = ParsedAux
    variable = weighted_s
    coupled_variables = 'device_s'
    use_xyzt = true
    expression = 'device_s * (x + 2 * y + 3 * z)'
  []
[]

[Postprocessors]
  # Zero when the device bridge reproduces the host bridge
  [max_abs_difference]
    type = ElementExtremeValue
    variable = difference
    value_type = max_abs
  []
  # Must reproduce the neml2_bridge test, where the same strain reaches NEML2 through the host
  [reference_magnitude]
    type = ElementExtremeValue
    variable = host_s
    value_type = max_abs
  []
  # Identifies the stress field itself, not just its extreme. Subdividing the mesh into subdomains
  # cannot change it, because the response is pointwise and the displacement is prescribed by
  # position, so the multiblock test golds the same value.
  [stress_moment]
    type = ElementIntegralVariablePostprocessor
    variable = weighted_s
  []
[]

[Outputs]
  csv = true
[]
