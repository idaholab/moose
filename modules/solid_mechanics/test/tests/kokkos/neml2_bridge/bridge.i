# Validates KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty, which reads a NEML2 output
# into a Kokkos material property without leaving the device.
#
# The same NEML2 stress is taken through both bridges in one run: the non-Kokkos bridge the NEML2
# action creates, which copies one batch entry per quadrature point into host memory, and the Kokkos
# bridge, which aliases the output's device storage and gathers it on the device. Both report the
# same stored Mandel component and the two are differenced, so the non-Kokkos path is the reference
# and the expected result is zero rather than a stored gold value.
#
# The displacement field is prescribed through auxiliary variables rather than solved, because only
# the constitutive update and the two bridges are under test.

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
  # Reference: the non-Kokkos bridge's host property
  [host_s]
    type = MaterialSymmetricRankTwoTensorAux
    variable = host_s
    property = 'neml2_stress'
    component = 5
  []
  # The Kokkos bridge's device property, same stored Mandel component
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
  # The Kokkos bridge, reading the same NEML2 output on the device
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
