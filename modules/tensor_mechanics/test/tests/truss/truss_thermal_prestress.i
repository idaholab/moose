[Mesh]
  type = GeneratedMesh
  dim = 1
  elem_type = EDGE
  nx = 3
[]

[GlobalParams]
  displacements = 'disp_x'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
 [./axial_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_over_l]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./forces]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./et]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./area]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./react_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./thermal_eig]
  [../]
  [./pre_stressing_eig]
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./load]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = 't'
  [../]
[]

[AuxKernels]
  [./axial_stress]
    type = MaterialRealAux
    property = axial_stress
    variable = axial_stress
  [../]
  [./e_over_l]
    type = MaterialRealAux
    property = e_over_l
    variable = e_over_l
  [../]
  [./forces]
    type = MaterialRealAux
    property = forces
    variable = forces
  [../]
  [./area]
    type = ConstantAux
    variable = area
    value = 1
    execute_on = 'initial timestep_begin'
  [../]
[]

[Postprocessors]
  [./s_xx]
    type = ElementIntegralMaterialProperty
    mat_prop = axial_stress
  [../]
  [./forces]
    type = ElementIntegralMaterialProperty
    mat_prop = forces
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-11
  l_max_its = 20
  dt = 1e-1
  num_steps = 3
[]

[Kernels]
  [./solid]
    type = StressDivergenceTruss
    component = 0
    variable = disp_x
    save_in = react_x
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityTruss
    youngs_modulus = 1e6
  [../]
  [./strain]
    type = ComputeIncrementalTrussStrain
    displacements = 'disp_x'
    area = area
    eigenstrain_names = 'thermal_eig pre_stressing_eig'
  [../]
  [./stress]
    type = ComputeTrussResultants
    area = area
  [../]
  [./thermal_eig]
    type = ComputeThermalExpansionEigenstrainTruss
    thermal_expansion_coeff = 1e-5
    temperature = 100
    stress_free_temperature = 0
    eigenstrain_name = thermal_eig
  [../]
  [./pre_stressing_eig]
    type = ComputePreStressEigenstrainTruss
    pre_stressing_strain = -1e-3
    eigenstrain_name = pre_stressing_eig
  [../]
[]

[Outputs]
  exodus = true
  csv = true
[]
