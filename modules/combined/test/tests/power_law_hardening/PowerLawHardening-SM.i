# This is a test of the isotropic power law hardening constitutive model.
# In this problem, a single Hex 8 element is fixed at the bottom and pulled at the top
# at a constant rate of 0.1.
# Before yield, stress = strain (=0.1*t) as youngs modulus is 1.0.
# The yield stress for this problem is 0.25 ( as strength coefficient is 0.5 and strain rate exponent is 0.5).
# Therefore, the material should start yielding at t = 2.5 seconds and then follow stress = K *pow(strain,n) or
# stress ~ 0.5*pow(0.1*t,0.5).

[Mesh]
  type = GeneratedMesh
  dim = 3
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

  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./total_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]


[Functions]
  [./top_pull]
    type = ParsedFunction
    value = t*(0.1)
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]


[AuxKernels]

  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]

  [./total_strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = total_strain_yy
    index = 1
  [../]
 []


[BCs]

  [./y_pull_function]
    type = FunctionPresetBC
    variable = disp_y
    boundary = top
    function = top_pull
  [../]

  [./x_bot]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]

  [./y_bot]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]

  [./z_bot]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]

[]

[Materials]
  [./vermont]
    type = SolidModel
    formulation = linear
    block = 0
    youngs_modulus = 1.0
    poissons_ratio = 0.3
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    constitutive_model = powerlaw
  [../]
  [./powerlaw]
    type = IsotropicPowerLawHardening
    block = 0
    strength_coefficient = 0.5 #K
    strain_hardening_exponent = 0.5 #n
    relative_tolerance = 1e-10
    absolute_tolerance = 1e-12
#    output_iteration_info = true
  [../]

[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time = 5.0
  dt = 0.25
[]

[Postprocessors]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./strain_yy]
    type = ElementAverageValue
    variable = total_strain_yy
  [../]
[]


[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
  file_base = PowerLawHardening_out
[]
