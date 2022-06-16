# Jacobian check for nonlinear, multi-surface plasticity
# This returns to the edge of Mohr Coulomb.
# This is a very nonlinear test and a delicate test because it perturbs around
# an edge of the yield function where some derivatives are not well defined
#
# Plasticity models:
# Mohr-Coulomb with cohesion = 40MPa, friction angle = 35deg, dilation angle = 5deg
# Tensile with strength = 1MPa
#
# Lame lambda = 1GPa.  Lame mu = 1.3GPa
#
# NOTE: The yield function tolerances here are set at 100-times what i would usually use
# This is because otherwise the test fails on the 'pearcey' architecture.
# This is because identical stress tensors yield slightly different eigenvalues
# (and hence return-map residuals) on 'pearcey' than elsewhere, which results in
# a different number of NR iterations are needed to return to the yield surface.
# This is presumably because of compiler internals, or the BLAS routines being
# optimised differently or something similar.

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
  [./int3]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int4]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int5]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int6]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int7]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int8]
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
  [./int3]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int3
    index = 3
  [../]
  [./int4]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int4
    index = 4
  [../]
  [./int5]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int5
    index = 5
  [../]
  [./int6]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int6
    index = 6
  [../]
  [./int7]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int7
    index = 7
  [../]
  [./int8]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    variable = int8
    index = 8
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
  [./max_int3]
    type = ElementExtremeValue
    variable = int3
    outputs = console
  [../]
  [./max_int4]
    type = ElementExtremeValue
    variable = int4
    outputs = console
  [../]
  [./max_int5]
    type = ElementExtremeValue
    variable = int5
    outputs = console
  [../]
  [./max_int6]
    type = ElementExtremeValue
    variable = int6
    outputs = console
  [../]
  [./max_int7]
    type = ElementExtremeValue
    variable = int7
    outputs = console
  [../]
  [./max_int8]
    type = ElementExtremeValue
    variable = int8
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
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 4E1
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 35
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 5
    convert_to_radians = true
  [../]
  [./mc]
    type = TensorMechanicsPlasticMohrCoulombMulti
    cohesion = mc_coh
    friction_angle = mc_phi
    dilation_angle = mc_psi
    yield_function_tolerance = 1.0E-4  # Note larger value
    shift = 1.0E-4                     # Note larger value
    internal_constraint_tolerance = 1.0E-7
  [../]

  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 1E2
  [../]
  [./tensile]
    type = TensorMechanicsPlasticTensileMulti
    tensile_strength = ts
    yield_function_tolerance = 1.0E-4  # Note larger value
    shift = 1.0E-4                     # Note larger value
    internal_constraint_tolerance = 1.0E-7
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '1.0E3 1.3E3'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '100.1 0.1 -0.2  0.1 0.9 0  -0.2 0 1.1'
    eigenstrain_name = ini_stress
  [../]
  [./multi]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-7

    plastic_models = 'tensile mc'
    max_NR_iterations = 5
    specialIC = 'rock'
    deactivation_scheme = 'safe'
    min_stepsize = 1
    max_stepsize_for_dumb = 1
    tangent_operator = nonlinear
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
  end_time = 1
[]



[Outputs]
  file_base = cto15
  exodus = false
[]
