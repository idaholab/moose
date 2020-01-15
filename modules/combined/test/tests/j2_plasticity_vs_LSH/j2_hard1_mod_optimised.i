# Test designed to compare results and active time between SH/LinearStrainHardening
# material vs TM j2 plastic user object. As number of elements increases, TM
# active time increases at a much higher rate than SM. Testing at 4x4x4
# (64 elements).
#
# plot vm_stress vs intnl to see constant hardening
#
# Original test located at:
# tensor_mechanics/tests/j2_plasticity/hard1.i

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 4
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vm_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./eq_pl_strain]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./intnl]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_internal_parameter
    variable = intnl
  [../]
  [./eq_pl_strain]
    type = RankTwoScalarAux
    rank_two_tensor = plastic_strain
    scalar_type = EffectiveStrain
    variable = eq_pl_strain
  [../]
  [./vm_stress]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = VonMisesStress
    variable = vm_stress
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./back]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = 't/60'
  [../]
[]

[UserObjects]
  [./str]
    type = TensorMechanicsHardeningConstant
    value = 2.4e2
  [../]
  [./j2]
    type = TensorMechanicsPlasticJ2
    yield_strength = str
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    #with E = 2.1e5 and nu = 0.3
    #Hooke's law: E-nu to Lambda-G
    C_ijkl = '121154 80769.2'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-9
    plastic_models = j2
    perform_finite_strain_rotations = false
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]


[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = NEWTON

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  #line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10
  l_tol = 1e-4
  start_time = 0.0
  end_time = 0.5
  dt = 0.5
[]

[Postprocessors]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./intnl]
    type = ElementAverageValue
    variable = intnl
  [../]
  [./eq_pl_strain]
    type = PointValue
    point = '0 0 0'
    variable = eq_pl_strain
  [../]
  [./vm_stress]
    type = PointValue
    point = '0 0 0'
    variable = vm_stress
  [../]
[]

[Outputs]
  csv = true
  print_linear_residuals = false
  perf_graph = true
[]
