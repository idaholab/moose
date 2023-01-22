[GlobalParams]
  displacements = 'disp_x disp_y'
  temperature = temp
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./total_stretch]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mechanical_stretch]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = NONORDINARY_STATE
    stabilization = BOND_HORIZON_I
    eigenstrain_names = thermal_strain
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]

  [./total_stretch]
    type = MaterialRealAux
    variable = total_stretch
    property = total_stretch
  [../]
  [./mechanical_stretch]
    type = MaterialRealAux
    variable = mechanical_stretch
    property = mechanical_stretch
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    boundary = 1003
    preset = false
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = 1000
    preset = false
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
  [./strain]
    type = ComputePlaneSmallStrainNOSPD
    stabilization = BOND_HORIZON_I
    eigenstrain_names = thermal_strain
    plane_strain = true
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 0.0002
    stress_free_temperature = 0.0
    eigenstrain_name = thermal_strain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  l_tol = 1e-8
  nl_rel_tol = 1e-10

  start_time = 0.0
  end_time = 1.0

  [./Quadrature]
    type = GAUSS_LOBATTO
    order = FIRST
  [../]
[]

[Outputs]
  exodus = true
  file_base = planestrain_thermomechanics_stretch_H1NOSPD
[]
