# Validates KokkosMOOSESymmetricRankTwoTensorToNEML2, which gathers a Kokkos material property into a
# NEML2 input variable on the device.
#
# The strain NEML2 consumes is produced by a Kokkos material from a vector displacement variable and
# handed to NEML2 by the Kokkos gatherer, replacing the host gatherer the NEML2 action would
# otherwise create. Everything downstream is unchanged from the neml2_bridge test, so the reported
# stress magnitude must reproduce that test's values: the constitutive problem is identical and only
# the route the strain takes into NEML2 differs.
#
# The model is stateful, so its old state is still gathered on the host through
# NEML2BatchIndexGenerator's numbering while the strain arrives through the Kokkos numbering. The
# second step is what tests that the two agree, since the old state is zero on the first one.
#
# The displacement field is prescribed through an auxiliary variable rather than solved, because only
# the strain gather and the constitutive update are under test. A solved displacement field would
# bring in a displaced mesh, which the Kokkos problem does not support.

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
    # The strain is supplied by the Kokkos gatherer below instead of a host gatherer
    input_kernels = 'neml2_strain'
  []
[]

[UserObjects]
  [neml2_strain]
    type = KokkosMOOSESymmetricRankTwoTensorToNEML2
    from_moose = 'mechanical_strain'
    to_neml2 = 'neml2_strain'
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
  # Must reproduce the neml2_bridge test, where the same strain reaches NEML2 through the host
  [reference_magnitude]
    type = ElementExtremeValue
    variable = host_s
    value_type = max_abs
  []
[]

[Outputs]
  csv = true
[]
