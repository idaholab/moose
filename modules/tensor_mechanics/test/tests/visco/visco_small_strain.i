[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  elem_type = HEX8
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

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = stress
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  [./strain_xx]
    type = RankTwoAux
    variable = strain_xx
    rank_two_tensor = total_strain
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  [./creep_strain_xx]
    type = RankTwoAux
    variable = creep_strain_xx
    rank_two_tensor = creep_strain
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  # [./axial_load]
  #   type = NeumannBC
  #   variable = disp_x
  #   boundary = right
  #   value    = 10e6
  # [../]
  [./axial_load]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value    = -1.97e-04
  [../]
[]

[Materials]
  [./kelvin_voigt]
    type = GeneralizedKelvinVoigtModel
    creep_modulus =   ' 1.515918662e3
                        5.319148936e9
                        6.150855892e1
                        6.861628402e1
                        4.484787600e1
                        1.050831891e9'
    creep_viscosity = ' 1.515918662e3
                        5.319148936e10
                        6.150855892e3
                        6.861628402e4
                        4.484787600e5
                        1.050830000e14'
    poisson_ratio = 0.2
    young_modulus = 3.368284903e10
  [../]
  [./stress]
    type = ComputeLinearViscoelasticStress
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[UserObjects]
  [./update]
    type = LinearViscoelasticityManager
    viscoelastic_model = kelvin_voigt
  [../]
[]

[Postprocessors]
  [./stress_xx]
    type = ElementAverageValue
    variable = stress_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./strain_xx]
    type = ElementAverageValue
    variable = strain_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./creep_strain_xx]
    type = ElementAverageValue
    variable = creep_strain_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  l_max_its  = 100
  l_tol      = 1e-8
  nl_max_its = 50
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8

  dtmin = 0.01
  # end_time = 100
  end_time = 17000
  [./TimeStepper]
    type = LogConstantDT
    first_dt = 0.1
    log_dt = 0.1
  [../]

[]

[Outputs]
  # file_base = visco_small_strain_out
  # exodus = true
  perf_graph     = true
  csv = true
  [./Console]
    type = Console
  [../]
  [./Exo]
    type = Exodus
    elemental_as_nodal = true
  [../]

[]
