# Exercises the `ComputeLagrangianStressCustomPK2` PK1 wrap (PK1 = F_ust * PK2) and its
# Jacobian. The published PK2 stress and dPK2/dF come from a test-only analytic Saint-Venant-
# Kirchhoff material (StVenantKirchhoffPK2Test), so the Jacobian tester sees the same
# numerical sensitivities the NEML2 crystal-plasticity wrap would produce.

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
  [pk2_source]
    type = StVenantKirchhoffPK2Test
    lambda = 40000.0
    mu = 67000.0
    pk2_name = test_pk2
    dpk2_dF_name = test_dpk2_dF
  []
  [stress]
    type = ComputeLagrangianStressCustomPK2
    custom_pk2_stress = test_pk2
    custom_pk2_jacobian = test_dpk2_dF
    large_kinematics = true
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
