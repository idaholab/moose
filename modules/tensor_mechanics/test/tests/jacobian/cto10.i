# checking jacobian for 3-plane linear plasticity using SimpleTester.
#
# This is like the test multi/six_surface14.i
# Plasticity models:
# SimpleTester0 with a = 0 and b = 1 and strength = 1
# SimpleTester1 with a = 1 and b = 0 and strength = 1
# SimpleTester2 with a = 1 and b = 1 and strength = 3
# SimpleTester3 with a = 0 and b = 1 and strength = 1.1
# SimpleTester4 with a = 1 and b = 0 and strength = 1.1
# SimpleTester5 with a = 1 and b = 1 and strength = 3.1
#
# Lame lambda = 0 (Poisson=0).  Lame mu = 0.5E6
#
# trial stress_yy = 2.1 and stress_zz = 3.0
#
# This is similar to three_surface14.i, and a description is found there.
# The result should be stress_zz=1=stress_yy, with internal0=2
# and internal1=1.1




[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]

[GlobalParams]
  block = 0
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]


[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[UserObjects]
  [./simple0]
    type = TensorMechanicsPlasticSimpleTester
    a = 0
    b = 1
    strength = 1
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
  [./simple1]
    type = TensorMechanicsPlasticSimpleTester
    a = 1
    b = 0
    strength = 1
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
  [./simple2]
    type = TensorMechanicsPlasticSimpleTester
    a = 1
    b = 1
    strength = 3
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
  [./simple3]
    type = TensorMechanicsPlasticSimpleTester
    a = 0
    b = 1
    strength = 1.1
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
  [./simple4]
    type = TensorMechanicsPlasticSimpleTester
    a = 1
    b = 0
    strength = 1.1
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
  [./simple5]
    type = TensorMechanicsPlasticSimpleTester
    a = 1
    b = 1
    strength = 3.1
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5E6'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '0 0 0  0 2.1 0  0 0 3.0'
    eigenstrain_name = ini_stress
  [../]
  [./multi]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-9
    plastic_models = 'simple0 simple1 simple2 simple3 simple4 simple5'
    tangent_operator = linear
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
[]
