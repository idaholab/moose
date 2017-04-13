#
# Test to exercise the exponential stress release
#
# Stress vs. strain should show a linear relationship until cracking,
#   an exponential stress release, a linear relationship back to zero
#   strain, a linear relationship with the original stiffness in
#   compression and then back to zero strain, a linear relationship
#   back to the exponential curve, and finally further exponential
#   stress release.
#

[GlobalParams]
  volumetric_locking_correction = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 1.0
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

  [./resid_x]
  [../]

  [./resid_y]
  [../]

  [./resid_z]
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

  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./damage]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./ref_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress0_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]

[Functions]
  [./displz]
    type = ParsedFunction
    value = ' 0.01 * t '
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    save_in_disp_x = resid_x
    save_in_disp_y = resid_y
    save_in_disp_z = resid_z
  [../]
[]


[AuxKernels]
  [./strain_xx]
    type = MaterialTensorAux
    variable = strain_xx
    tensor = total_strain
    index = 0
  [../]

  [./strain_yy]
    type = MaterialTensorAux
    variable = strain_yy
    tensor = total_strain
    index = 1
  [../]

  [./strain_zz]
    type = MaterialTensorAux
    variable = strain_zz
    tensor = total_strain
    index = 2
  [../]

  [./stress_xx]
    type = MaterialTensorAux
    variable = stress_xx
    tensor = stress
    index = 0
  [../]

  [./stress_yy]
    type = MaterialTensorAux
    variable = stress_yy
    tensor = stress
    index = 1
  [../]

  [./stress_zz]
    type = MaterialTensorAux
    variable = stress_zz
    tensor = stress
    index = 2
  [../]

  [./stress_xy]
    type = MaterialTensorAux
    variable = stress_xy
    tensor = stress
    index = 3
  [../]

  [./stress_yz]
    type = MaterialTensorAux
    variable = stress_yz
    tensor = stress
    index = 4
  [../]

  [./stress_zx]
    type = MaterialTensorAux
    variable = stress_zx
    tensor = stress
    index = 5
  [../]

  [./stress0_zz]
    type = MaterialTensorAux
    variable = stress0_zz
    tensor = stress_undamaged
    index = 2
  [../]

  [./damage]
    type = MaterialStdVectorAux
    variable = damage
    property = intvar
    index = 0
  [../]

  [./ref_stress]
    type = MaterialStdVectorAux
    variable = ref_energy
    property = intvar
    index = 1
  [../]

[]


[BCs]
  [./pull_z]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = displz
  [../]
  [./fix_x]
    type = PresetBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  [../]
  [./fix_y]
    type = PresetBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  [../]
  [./fix_z]
    type = PresetBC
    variable = disp_z
    boundary = 'back'
    value = 0.0
  [../]
[]

[Materials]
  [./elastic]
    type = SolidModel
    block = 0
    youngs_modulus = 186.5e9
    poissons_ratio = .316
    formulation = linear
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    constitutive_model = crack
  [../]

  [./crack]
    type = RateDepSmearIsoCrackModel
    block = 0
    critical_energy = 1e6
    ref_damage_rate = 1e-2
    tol = 1e-5
    maxiter = 100
    exponent = 0.75
    nstate = 2
    intvar_incr_tol = 10.0 #large value to avoid cutback
  [../]
[]

[Executioner]
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type'
  petsc_options_value = '101                lu'

  line_search = 'none'
  nl_max_its = 20
  nl_rel_tol = 1.5e-11
  nl_abs_tol = 1e-8

  dt = 0.01
  dtmin = 0.01
  end_time = 0.5
[]

[Outputs]
  exodus = true
  csv = true
  gnuplot = true
[]

[Postprocessors]
  [./stress_xx]
    type = ElementAverageValue
    variable = stress_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./stress0_zz]
    type = ElementAverageValue
    variable = stress0_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./strain_xx]
    type = ElementAverageValue
    variable = strain_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./strain_yy]
    type = ElementAverageValue
    variable = strain_yy
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./strain_zz]
    type = ElementAverageValue
    variable = strain_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./damage]
    type = ElementAverageValue
    variable = damage
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./ref_energy]
    type = ElementAverageValue
    variable = ref_energy
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./react_x]
    type = NodalSum
    variable = resid_x
    boundary = left
  [../]
  [./react_y]
    type = NodalSum
    variable = resid_y
    boundary = bottom
  [../]
  [./react_z]
    type = NodalSum
    variable = resid_z
    boundary = back
  [../]
[]
