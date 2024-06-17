# checking jacobian for 3-plane linear plasticity using SimpleTester.
#
# This is like the test multi/three_surface12.i
# Plasticity models:
# SimpleTester0 with a = 0 and b = 1 and strength = 1
# SimpleTester1 with a = 1 and b = 0 and strength = 1
# SimpleTester2 with a = 1 and b = 1 and strength = 1.5
#
# Lame lambda = 0 (Poisson=0).  Lame mu = 0.5E6
#
# trial stress_yy = 0.15 and stress_zz = 1.5
#
# Then SimpleTester0 and SimpleTester1 should activate and the algorithm will return to
# stress_zz=1=stress_yy
# internal0 should be 0.5 and internal1 should be 0.5



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
  [SolidMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[UserObjects]
  [./simple0]
    type = SolidMechanicsPlasticSimpleTester
    a = 0
    b = 1
    strength = 1
    yield_function_tolerance = 1.0E-9
    internal_constraint_tolerance = 1.0E-9
  [../]
  [./simple1]
    type = SolidMechanicsPlasticSimpleTester
    a = 1
    b = 0
    strength = 1
    yield_function_tolerance = 1.0E-9
    internal_constraint_tolerance = 1.0E-9
  [../]
  [./simple2]
    type = SolidMechanicsPlasticSimpleTester
    a = 1
    b = 1
    strength = 1.5
    yield_function_tolerance = 1.0E-9
    internal_constraint_tolerance = 1.0E-9
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5E6'
  [../]
  [./strain]
    type = ComputeIncrementalStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '0 0 0  0 0.15 0  0 0 1.5'
    eigenstrain_name = ini_stress
  [../]
  [./multi]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-9
    plastic_models = 'simple0 simple1 simple2'
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
