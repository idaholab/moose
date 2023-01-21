# This test allows comparison of simulation and analytical solution for a misfitting precipitate
# using ComputeVariableEigenstrain for the simulation and the InclusionProperties material
# for the analytical solution. Increasing mesh resolution will improve agreement.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  xmax = 1.5
  ymax = 1.5
  elem_type = QUAD4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
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
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[AuxVariables]
  [./s11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s12_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s22_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s11_an]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s12_an]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s22_an]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e12_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e22_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e11_an]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e12_an]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e22_an]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./fel_an]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c]
  [../]
[]

[AuxKernels]
  [./matl_s11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = s11_aux
  [../]
  [./matl_s12]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
    variable = s12_aux
  [../]
  [./matl_s22]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = s22_aux
  [../]
  [./matl_s11_an]
    type = RankTwoAux
    rank_two_tensor = stress_an
    index_i = 0
    index_j = 0
    variable = s11_an
  [../]
  [./matl_s12_an]
    type = RankTwoAux
    rank_two_tensor = stress_an
    index_i = 0
    index_j = 1
    variable = s12_an
  [../]
  [./matl_s22_an]
    type = RankTwoAux
    rank_two_tensor = stress_an
    index_i = 1
    index_j = 1
    variable = s22_an
  [../]
  [./matl_e11]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 0
    variable = e11_aux
  [../]
  [./matl_e12]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 1
    variable = e12_aux
  [../]
  [./matl_e22]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
    variable = e22_aux
  [../]
  [./matl_e11_an]
    type = RankTwoAux
    rank_two_tensor = strain_an
    index_i = 0
    index_j = 0
    variable = e11_an
  [../]
  [./matl_e12_an]
    type = RankTwoAux
    rank_two_tensor = strain_an
    index_i = 0
    index_j = 1
    variable = e12_an
  [../]
  [./matl_e22_an]
    type = RankTwoAux
    rank_two_tensor = strain_an
    index_i = 1
    index_j = 1
    variable = e22_an
  [../]
  [./matl_fel_an]
    type = MaterialRealAux
    variable = fel_an
    property = fel_an_mat
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./var_dependence]
    type = DerivativeParsedMaterial
    block = 0
    expression = 0.005*c^2
    coupled_variables = c
    outputs = exodus
    output_properties = 'var_dep'
    f_name = var_dep
    enable_jit = true
    derivative_order = 2
  [../]
  [./eigenstrain]
    type = ComputeVariableEigenstrain
    block = 0
    eigen_base = '1 1 0 0 0 0'
    prefactor = var_dep
    args = c
    eigenstrain_name = eigenstrain
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]
  [./analytical]
    type = InclusionProperties
    a = 0.1
    b = 0.1
    lambda = 1
    mu = 1
    misfit_strains = '0.005 0.005'
    strain_name = strain_an
    stress_name = stress_an
    energy_name = fel_an_mat
  [../]
[]

[BCs]
  active = 'left_x bottom_y'
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'
  l_max_its = 30
  nl_max_its = 10
  nl_rel_tol = 1.0e-10
[]

[Outputs]
  exodus = true
[]

[ICs]
  [./c_IC]
    int_width = 0.075
    x1 = 0
    y1 = 0
    radius = 0.1
    outvalue = 0
    variable = c
    invalue = 1
    type = SmoothCircleIC
  [../]
[]
