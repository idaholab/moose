# Exercises the `ComputeLagrangianCauchyCustomStress` Cauchy -> PK1 wrap and its
# Jacobian. The published cauchy stress and dsigma/d(dL) come from a test-only
# hypoelastic isotropic material (IsotropicCauchyStressTest), so the Jacobian tester
# sees the same numerical sensitivities the approx-kinematics NEML2 wrap would produce
# (NEML2 derivative: state/internal/full_cauchy_stress / forces/spatial_deformation_gradient_increment).

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
  stabilize_strain = false
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
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

[Functions]
  [ux]
    type = ParsedFunction
    expression = '0.01 * x + 0.005 * y'
  []
  [uy]
    type = ParsedFunction
    expression = '-0.003 * x + 0.008 * y + 0.004 * z'
  []
  [uz]
    type = ParsedFunction
    expression = '0.002 * y + 0.006 * z'
  []
[]

[BCs]
  [bx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'left right top bottom front back'
    function = ux
    preset = true
  []
  [by]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'left right top bottom front back'
    function = uy
    preset = true
  []
  [bz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'left right top bottom front back'
    function = uz
    preset = true
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[Materials]
  [strain]
    type = ComputeLagrangianStrain
  []
  [cauchy_source]
    type = IsotropicCauchyStressTest
    lambda = 40000.0
    mu = 67000.0
    sigma_name = test_cauchy
    dsigma_d_dL_name = test_dcauchy_d_dL
  []
  [stress]
    type = ComputeLagrangianCauchyCustomStress
    custom_cauchy_stress = test_cauchy
    custom_cauchy_jacobian = test_dcauchy_d_dL
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-10
  dt = 1.0
  num_steps = 1
[]
