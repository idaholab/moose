#
# Test for effective strain calculation.
# Boundary conditions from NAFEMS test NL1
#
# This is not a verification test. This is the creep analog of the same test
# in the elas_plas directory. Instead of using the IsotropicPlasticity
# material model this test uses the PowerLawCreep material model.
#
[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
  temp = temp
  order = FIRST
  family = LAGRANGE
  volumetric_locking_correction = true
  block = 1
[]

[Mesh]
  file = one_elem2.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./temp]
    initial_condition = 600.0
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./pressure]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tot_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tot_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tot_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./eff_creep_strain]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
  [./heat_ie]
    type = HeatConductionTimeDerivative
    variable = temp
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
    execute_on = timestep_end
  [../]
  [./pressure]
    type = MaterialTensorAux
    tensor = stress
    variable =pressure
    quantity = hydrostatic
    execute_on = timestep_end
  [../]
  [./elastic_strain_xx]
    type = MaterialTensorAux
    tensor = elastic_strain
    variable = elastic_strain_xx
    index = 0
  [../]
  [./elastic_strain_yy]
    type = MaterialTensorAux
    tensor = elastic_strain
    variable = elastic_strain_yy
    index = 1
  [../]
  [./elastic_strain_zz]
    type = MaterialTensorAux
    tensor = elastic_strain
    variable = elastic_strain_zz
    index = 2
  [../]
  [./creep_strain_xx]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_xx
    index = 0
  [../]
  [./creep_strain_yy]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_yy
    index = 1
  [../]
  [./creep_strain_zz]
    type = MaterialTensorAux
    tensor = creep_strain
    variable = creep_strain_zz
    index = 2
  [../]
  [./tot_strain_xx]
    type = MaterialTensorAux
    tensor = total_strain
    variable = tot_strain_xx
    index = 0
  [../]
  [./tot_strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = tot_strain_yy
    index = 1
  [../]
  [./tot_strain_zz]
    type = MaterialTensorAux
    tensor = total_strain
    variable = tot_strain_zz
    index = 2
  [../]
  [./eff_creep_strain]
    type = MaterialRealAux
    property = effective_creep_strain
    variable = eff_creep_strain
  [../]
[]

[Functions]
  [./appl_dispy]
    type = PiecewiseLinear
    x = '0     1.0     2.0'
    y = '0.0 0.25e-4 0.50e-4'
  [../]
[]

[BCs]
  [./side_x]
    type = DirichletBC
    variable = disp_x
    boundary = 101
    value = 0.0
  [../]
  [./origin_x]
    type = DirichletBC
    variable = disp_x
    boundary = 103
    value = 0.0
  [../]
  [./bot_y]
    type = DirichletBC
    variable = disp_y
    boundary = 102
    value = 0.0
  [../]
  [./origin_y]
    type = DirichletBC
    variable = disp_y
    boundary = 103
    value = 0.0
  [../]
  [./top_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 1
    function = appl_dispy
  [../]
  [./temp_fix]
    type = DirichletBC
    variable = temp
    boundary = '1 2'
    value = 600.0
  [../]
[]

[Materials]
  [./stiff]
    type = SolidModel
    block = 1
    youngs_modulus = 250e9
    poissons_ratio = 0.25
    formulation = NonlinearPlaneStrain
    constitutive_model = powerlawcrp
    increment_calculation = Eigen
  [../]
  [./powerlawcrp]
    type = PowerLawCreepModel
    block = 1
    coefficient = 3.125e-14
    n_exponent = 5.0
    m_exponent = 0.0
    activation_energy = 0.0
  [../]

  [./thermal]
    type = HeatConductionMaterial
    block = 1
    specific_heat = 1.0
    thermal_conductivity = 100.
  [../]

  [./density]
    type = Density
    block = 1
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-4
  l_max_its = 100
  nl_max_its = 20

  dt = 1.0
  start_time = 0.0
  num_steps = 100
  end_time = 2.0
[]

[Postprocessors]
  [./stress_xx]
    type = ElementAverageValue
    variable = stress_xx
  [../]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./stress_xy]
    type = ElementAverageValue
    variable = stress_xy
  [../]
  [./vonmises]
    type = ElementAverageValue
    variable = vonmises
  [../]
  [./pressure]
    type = ElementAverageValue
    variable = pressure
  [../]
  [./el_strain_xx]
    type = ElementAverageValue
    variable = elastic_strain_xx
  [../]
  [./el_strain_yy]
    type = ElementAverageValue
    variable = elastic_strain_yy
  [../]
  [./el_strain_zz]
    type = ElementAverageValue
    variable = elastic_strain_zz
  [../]
  [./crp_strain_xx]
    type = ElementAverageValue
    variable = creep_strain_xx
  [../]
  [./crp_strain_yy]
    type = ElementAverageValue
    variable = creep_strain_yy
  [../]
  [./crp_strain_zz]
    type = ElementAverageValue
    variable = creep_strain_zz
  [../]
  [./eff_creep_strain]
    type = ElementAverageValue
    variable = eff_creep_strain
  [../]
  [./tot_strain_xx]
    type = ElementAverageValue
    variable = tot_strain_xx
  [../]
  [./tot_strain_yy]
    type = ElementAverageValue
    variable = tot_strain_yy
  [../]
  [./tot_strain_zz]
    type = ElementAverageValue
    variable = tot_strain_zz
  [../]
  [./disp_x1]
    type = NodalVariableValue
    nodeid = 0
    variable = disp_x
  [../]
  [./disp_x4]
    type = NodalVariableValue
    nodeid = 3
    variable = disp_x
  [../]
  [./disp_y1]
    type = NodalVariableValue
    nodeid = 0
    variable = disp_y
  [../]
  [./disp_y4]
    type = NodalVariableValue
    nodeid = 3
    variable = disp_y
  [../]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  exodus = true
  file_base=creep_nl1_out
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]
