# The problem of cpu_small_strain.i, assembled instead through the Kokkos material and kernel
# chain, against the same gold. A single LAGRANGE_VEC variable carries all three displacement
# components, so one kernel replaces the three component kernels of the non-Kokkos input.
#
# The displacement components are extracted into auxiliary variables only so that the reported
# norms carry the same names as in the reference input, letting both inputs share one gold file.

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
  [disp]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[Kernels]
  [stress_divergence]
    type = KokkosVectorStressDivergence
    variable = disp
  []
[]

[Materials]
  [elasticity_tensor]
    type = KokkosComputeIsotropicElasticityTensor
    youngs_modulus = 2.5
    poissons_ratio = 0.25
  []
  [strain]
    type = KokkosComputeSmallStrain
    displacements = disp
  []
  [stress]
    type = KokkosComputeLinearElasticStress
  []
[]

[BCs]
  [fixed]
    type = KokkosVectorDirichletBC
    variable = disp
    boundary = left
    values = '0 0 0'
  []
  [pull]
    type = KokkosVectorDirichletBC
    variable = disp
    boundary = right
    values = '0.001 0.002 -0.003'
  []
[]

[AuxVariables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [disp_x]
    type = VectorVariableComponentAux
    variable = disp_x
    vector_variable = disp
    component = x
  []
  [disp_y]
    type = VectorVariableComponentAux
    variable = disp_y
    vector_variable = disp
    component = y
  []
  [disp_z]
    type = VectorVariableComponentAux
    variable = disp_z
    vector_variable = disp
    component = z
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Postprocessors]
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
[]

[Outputs]
  csv = true
[]
