#
# Test for effective strain calculation.
# Boundary conditions from NAFEMS test NL1
#
# This is not a verification test. The boundary conditions are applied such
# that the first step generates only elastic stresses. The second and third
# steps generate plastic deformation and the effective strain should be
# increasing throughout the run.
#
[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
  order = FIRST
  family = LAGRANGE
  volumetric_locking_correction = true
[]

[Mesh]#Comment
  file = one_elem2.e
  displacements = 'disp_x disp_y'
[] # Mesh

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[] # Variables

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
  [./plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_strain_zz]
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
  [./eff_plastic_strain]
    order = CONSTANT
    family = MONOMIAL
  [../]
[] # AuxVariables

[SolidMechanics]
  [./solid]
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
    variable = pressure
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
  [./plastic_strain_xx]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_xx
    index = 0
  [../]
  [./plastic_strain_yy]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_yy
    index = 1
  [../]
  [./plastic_strain_zz]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_zz
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
  [./eff_plastic_strain]
    type = MaterialRealAux
    property = effective_plastic_strain
    variable = eff_plastic_strain
  [../]
[] # AuxKernels

[Functions]
  [./appl_dispy]
    type = PiecewiseLinear
    x = '0     1.0     2.0     3.0'
    y = '0.0 0.208e-4 0.50e-4 1.00e-4'
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
[] # BCs

[Materials]
  [./stiffStuff1]
    type = SolidModel
    block = 1
    youngs_modulus = 250e9
    poissons_ratio = 0.25
    constitutive_model = isoplas
    formulation = NonlinearPlaneStrain
  [../]
  [./isoplas]
    type = IsotropicPlasticity
    block = 1
    yield_stress = 5e6
    hardening_constant = 0.0
  [../]
[] # Materials

[Executioner]
  type = Transient
  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-4
  l_max_its = 100
  nl_max_its = 20

  dt = 1.0
  start_time = 0.0
  num_steps = 100
  end_time = 3.0
[] # Executioner

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
  [./pl_strain_xx]
    type = ElementAverageValue
    variable = plastic_strain_xx
  [../]
  [./pl_strain_yy]
    type = ElementAverageValue
    variable = plastic_strain_yy
  [../]
  [./pl_strain_zz]
    type = ElementAverageValue
    variable = plastic_strain_zz
  [../]
  [./eff_plastic_strain]
    type = ElementAverageValue
    variable = eff_plastic_strain
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
  csv = true
  file_base=elas_plas_nl1_out
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[] # Outputs
