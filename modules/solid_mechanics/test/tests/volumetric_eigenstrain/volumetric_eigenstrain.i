# This tests the ability of the SolidMechanics system to exactly recover
# a specified volumetric expansion.
# This model applies volumetric strain that ramps from 0 to 2 to a unit cube
# and computes the final volume, which should be exactly 3.  Note that the default
# Taylor expansion option for increment_calculation gives a small (~4%) error
# with this very large incremental strain, but increment_calculation = Eigen
# gives the exact solution.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./volumetric_strain]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./temp]
    initial_condition = 0
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]
  [./volumetric_strain]
    type = MaterialTensorAux
    tensor = total_strain
    variable = volumetric_strain
    quantity = VolumetricStrain
  [../]
  [./temp]
    type = FunctionAux
    variable = temp
    function = pres_temp
  [../]
[]

[Functions]
  [pres_temp]
    type = PiecewiseLinear
    # Because SolidMechanics does not offer a way to directly prescribe the
    # volumetric eigenstrain, we do this by prescribing a thermal strain.  The
    # CTE of the material is set to 1, so the numbers here are the linear strains
    # that are applied. The engineering linear strains that give a volumetric
    # strain equal to 1 at time 1 and 2 at time 2 are commented out below. These
    # are converted to log strains as log(1+linear_strain):
    xy_data = '0 0
               1 0.23104906018664598
               2 0.3662040962227044'

    # Linear strains that these are computed from:
    # xy_data = '0 0
    #            1 0.25992104989487
    #            2 0.44224957030741'
  []
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./back]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./linelast]
    type = Elastic
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    increment_calculation = Eigen
    block = 0
    thermal_expansion = 1.0
    stress_free_temperature = 0.0
    temp = temp
  [../]
[]

[Postprocessors]
  [./vol]
    type = VolumePostprocessor
    use_displaced_mesh = true
    execute_on = 'initial timestep_end'
  [../]
  [./volumetric_strain]
    type = ElementalVariableValue
    variable = volumetric_strain
    elementid = 0
  [../]
  [./disp_right]
    type = NodalMaxValue
    variable = disp_x
    boundary = right
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  l_max_its = 100
  l_tol = 1e-4
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12

  start_time = 0.0
  end_time = 2.0
  dt = 1.0
[]

[Outputs]
  exodus = true
  csv = true
[]
