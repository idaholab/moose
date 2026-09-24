# Validates the Kokkos NEML2 gather and retrieve objects under a strain that varies in space, which
# is what gives a comparison of the two bridges any power.
#
# The prescribed displacement is quadratic in position, so its gradient differs from element to
# element. Under an affine displacement the gradient is spatially constant, every quadrature point
# carries the same strain and the same stress, and any permutation of the NEML2 batch is invisible to
# every comparison that can be made. Keep the displacement non-affine.
#
# On a single subdomain NEML2BatchLayout's numbering coincides with NEML2BatchIndexGenerator's, so the
# non-Kokkos bridge is a valid independent reference here and the two must agree pointwise. multiblock.i
# takes the same problem onto two subdomains, where that coincidence ends, and reproduces the stress
# field golded here.
#
# The displacement field is prescribed through an auxiliary variable rather than solved, because only
# the gather, the constitutive update and the retrieve are under test. A solved displacement field
# would bring in a displaced mesh, which the Kokkos problem does not support.

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
  # Quadratic in position, so that the strain varies from element to element
  [ramp]
    type = ParsedVectorFunction
    expression_x = '0.002 * t * x * x'
    expression_y = '-0.001 * t * y * y + 0.0005 * t * x * z'
    expression_z = '0.0003 * t * z * z + 0.0004 * t * x * y'
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
  [device_magnitude]
    type = ElementExtremeValue
    variable = device_s
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
