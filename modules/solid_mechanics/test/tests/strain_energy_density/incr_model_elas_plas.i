# Single element test to check the strain energy density calculation

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  disp_x = disp_x
  disp_y = disp_y
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 2
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./stress_xx]      # stress aux variables are defined for output; this is a way to get integration point variables to the output file
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
  [./vonmises]
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
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./SED]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./rampConstantUp]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = -100
  [../]
  [./ramp_disp_y]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 6.8e-6 1.36e-5'
  [../]
[]

[SolidMechanics]
  [./solid]
  [../]
[]

[AuxKernels]
  [./stress_xx]               # computes stress components for output
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep_end     # for efficiency, only compute at the end of a timestep
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    execute_on = timestep_end
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep_end
  [../]
  [./vonmises]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises
    quantity = vonmises
    execute_on = timestep_end
  [../]
  [./elastic_strain_xx]
    type = MaterialTensorAux
    tensor = elastic_strain
    variable = elastic_strain_xx
    index = 0
    execute_on = timestep_end
  [../]
  [./elastic_strain_yy]
    type = MaterialTensorAux
    tensor = elastic_strain
    variable = elastic_strain_yy
    index = 1
    execute_on = timestep_end
  [../]
  [./elastic_strain_zz]
    type = MaterialTensorAux
    tensor = elastic_strain
    variable = elastic_strain_zz
    index = 2
    execute_on = timestep_end
  [../]
  [./plastic_strain_xx]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_xx
    index = 0
    execute_on = timestep_end
  [../]
  [./plastic_strain_yy]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_yy
    index = 1
    execute_on = timestep_end
  [../]
  [./plastic_strain_zz]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_zz
    index = 2
    execute_on = timestep_end
  [../]
  [./strain_xx]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_xx
    index = 0
    execute_on = timestep_end
  [../]
  [./strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_yy
    index = 1
    execute_on = timestep_end
  [../]
  [./strain_zz]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_zz
    index = 2
    execute_on = timestep_end
  [../]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  [../]
  [./top_disp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top'
    function = ramp_disp_y
  [../]
[]

[Materials]
  [./stiffStuff]
    type = SolidModel
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 30e6
    poissons_ratio = 0.3
    formulation = NonlinearPlaneStrain
    compute_JIntegral = true
    constitutive_model = isoplas
  [../]
  [./isoplas]
    type = IsotropicPlasticity
    block = 0
    yield_stress = 1e2
    hardening_constant = 0.0
  [../]
[]

[Executioner]
   type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

   l_max_its = 50
   nl_max_its = 20
   nl_abs_tol = 3e-7
   nl_rel_tol = 1e-12
   l_tol = 1e-2

   start_time = 0.0
   dt = 1

   end_time = 2
   num_steps = 2
[]

[Postprocessors]
  [./epxx]
    type = ElementalVariableValue
    variable = elastic_strain_xx
    elementid = 0
  [../]
  [./epyy]
    type = ElementalVariableValue
    variable = elastic_strain_yy
    elementid = 0
  [../]
  [./epzz]
    type = ElementalVariableValue
    variable = elastic_strain_zz
    elementid = 0
  [../]
  [./eplxx]
    type = ElementalVariableValue
    variable = plastic_strain_xx
    elementid = 0
  [../]
  [./eplyy]
    type = ElementalVariableValue
    variable = plastic_strain_yy
    elementid = 0
  [../]
  [./eplzz]
    type = ElementalVariableValue
    variable = plastic_strain_zz
    elementid = 0
  [../]
  [./etxx]
    type = ElementalVariableValue
    variable = strain_xx
    elementid = 0
  [../]
  [./etyy]
    type = ElementalVariableValue
    variable = strain_yy
    elementid = 0
  [../]
  [./etzz]
    type = ElementalVariableValue
    variable = strain_zz
    elementid = 0
  [../]
  [./sigxx]
    type = ElementAverageValue
    variable = stress_xx
  [../]
  [./sigyy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./sigzz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./SED]
    type = ElementAverageValue
    variable = SED
  [../]
[]

[Outputs]
  exodus = true
  csv = true
[]
