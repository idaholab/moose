# Isotropic hardening plasticity on a small block, solved with the constitutive update evaluated by
# NEML2 through the non-Kokkos bridge and assembled through the non-Kokkos kernel chain. This input
# produces the reference solution that kokkos_neml2_solve.i must reproduce, so the two discretize the
# same problem: the same mesh, the same NEML2 model and the same prescribed displacements.
#
# The block is clamped on one end and the other end is pulled axially and transversely, so the stress
# field is nonuniform and the material yields progressively over the steps. The first step is
# elastic throughout, and the later ones carry the plastic state from step to step through the old
# state the model takes as input.
#
# StressDivergenceTensors assembles the same weak form as KokkosSymmetricVectorStressDivergence,
# (sigma, grad psi) with the stress and tangent taken as given, and ComputeSmallStrain evaluates the
# same total strain as KokkosSymmetricComputeSmallStrain, so the two chains agree to solver tolerance.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 2
    nx = 4
    ny = 2
    nz = 2
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [stress_divergence_x]
    type = StressDivergenceTensors
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    component = 0
  []
  [stress_divergence_y]
    type = StressDivergenceTensors
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    component = 1
  []
  [stress_divergence_z]
    type = StressDivergenceTensors
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    component = 2
  []
[]

[BCs]
  [fixed_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [fixed_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [fixed_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0
  []
  [pull_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = '5e-5 * t'
  []
  [pull_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = right
    function = '2.5e-5 * t'
  []
  [pull_z]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 0
  []
[]

[Functions]
  [position_weight]
    type = ParsedFunction
    expression = 'x + 2 * y + 3 * z'
  []
[]

[NEML2]
  input = '../../neml2/plasticity/isoharden_neml2.i'
  [all]
    model = 'model'
    device = 'cpu'
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
  # StressDivergenceTensors reads the stress and tangent in their dense forms
  [convert_stress]
    type = SymmetricRankTwoTensorToRankTwoTensor
    from = 'neml2_stress'
    to = 'stress'
  []
  [convert_jacobian]
    type = SymmetricRankFourTensorToRankFourTensor
    from = 'dneml2_stress/dneml2_strain'
    to = 'Jacobian_mult'
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  # Named apart from the equivalent_plastic_strain property: the NEML2 action gathers a scalar model
  # input from a variable of the same name in preference to a material property, so an elemental
  # variable of that name would supply the old state as an element average
  [eqps]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_xx]
    type = MaterialSymmetricRankTwoTensorAux
    variable = stress_xx
    property = 'neml2_stress'
    component = 0
    execute_on = 'TIMESTEP_END'
  []
  [eqps]
    type = MaterialRealAux
    variable = eqps
    property = 'equivalent_plastic_strain'
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  # A direct solve keeps the comparison between the two chains limited by assembly roundoff
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 4
  dt = 1
[]

[Postprocessors]
  # Norms and integrals over the whole domain, rather than extrema, so that the comparison is
  # sensitive to the interior solution
  [disp_x_l2]
    type = ElementL2Norm
    variable = disp_x
  []
  [disp_y_l2]
    type = ElementL2Norm
    variable = disp_y
  []
  [disp_z_l2]
    type = ElementL2Norm
    variable = disp_z
  []
  # Weighting by position makes this integral sensitive to the stress field being permuted among
  # elements, which a plain integral is not
  [stress_moment]
    type = ElementIntegralFunctorPostprocessor
    functor = stress_xx
    prefactor = position_weight
  []
  [plastic_strain_integral]
    type = ElementIntegralVariablePostprocessor
    variable = eqps
  []
  # Reported to show that the loading reaches the plastic response, which the integral above cannot
  # distinguish from a small uniform value
  [max_equivalent_plastic_strain]
    type = ElementExtremeValue
    variable = eqps
  []
[]

[Outputs]
  csv = true
[]
