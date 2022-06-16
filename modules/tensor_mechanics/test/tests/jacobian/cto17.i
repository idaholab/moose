# Jacobian check for nonlinear, multi-surface plasticity.
# Returns to the plane of the tensile yield surface
#
# Plasticity models:
# Tensile with strength = 1MPa softening to 0.5MPa in 2E-2 strain
#
# Lame lambda = 0.5GPa.  Lame mu = 1GPa
#
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



[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./linesearch]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ld]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./constr_added]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int0]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./linesearch]
    type = MaterialRealAux
    property = plastic_linesearch_needed
    variable = linesearch
  [../]
  [./ld]
    type = MaterialRealAux
    property = plastic_linear_dependence_encountered
    variable = ld
  [../]
  [./constr_added]
    type = MaterialRealAux
    property = plastic_constraints_added
    variable = constr_added
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
  [./int0]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int0
    index = 0
  [../]
  [./int1]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int1
    index = 1
  [../]
  [./int2]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int2
    index = 2
  [../]
[]

[Postprocessors]
  [./max_int0]
    type = ElementExtremeValue
    variable = int0
    outputs = console
  [../]
  [./max_int1]
    type = ElementExtremeValue
    variable = int1
    outputs = console
  [../]
  [./max_int2]
    type = ElementExtremeValue
    variable = int2
    outputs = console
  [../]
  [./max_iter]
    type = ElementExtremeValue
    variable = iter
    outputs = console
  [../]
  [./av_linesearch]
    type = ElementAverageValue
    variable = linesearch
    outputs = 'console'  [../]
  [./av_ld]
    type = ElementAverageValue
    variable = ld
    outputs = 'console'  [../]
  [./av_constr_added]
    type = ElementAverageValue
    variable = constr_added
    outputs = 'console'  [../]
  [./av_iter]
    type = ElementAverageValue
    variable = iter
    outputs = 'console'  [../]
[]


[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningCubic
    value_0 = 1
    value_residual = 0.5
    internal_limit = 2E-2
  [../]
  [./tensile]
    type = TensorMechanicsPlasticTensileMulti
    tensile_strength = ts
    yield_function_tolerance = 1.0E-6  # Note larger value
    shift = 1.0E-6                     # Note larger value
    internal_constraint_tolerance = 1.0E-7
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0.5E3 1E3'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '-1 0.1 0.2  0.1 15 -0.3  0.2 -0.3 0'
    eigenstrain_name = ini_stress
  [../]
  [./multi]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-7

    plastic_models = 'tensile'
    max_NR_iterations = 5
    deactivation_scheme = 'safe'
    min_stepsize = 1
    tangent_operator = nonlinear
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
[]


[Outputs]
  file_base = cto17
  exodus = false
[]
