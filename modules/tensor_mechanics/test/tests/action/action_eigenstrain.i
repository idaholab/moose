# The primary purpose of this test is to verify that the ability to combine
# multiple eigenstrains works correctly.  It should behave identically to the
# constant_expansion_coeff.i model in the thermal_expansion directory. Instead
# of having the eigenstrain names passed directly to the TensorMechanics MasterAction,
# the MasterAction should be able to extract the necessary eigenstrains and apply
# to their respective blocks without reduncacy.

# This test involves only thermal expansion strains on a 2x2x2 cube of approximate
# steel material.  An initial temperature of 25 degrees C is given for the material,
# and an auxkernel is used to calculate the temperature in the entire cube to
# raise the temperature each time step.  After the first timestep,in which the
# temperature jumps, the temperature increases by 6.25C each timestep.
# The thermal strain increment should therefore be
#     6.25 C * 1.3e-5 1/C = 8.125e-5 m/m.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [./temp]
  [../]
[]

[Functions]
  [./temperature_load]
    type = ParsedFunction
    expression = t*(500.0)+300.0
  [../]
[]

[Modules/TensorMechanics/Master]
  [./solid]
    strain = SMALL
    incremental = true
    add_variables = true
    automatic_eigenstrain_names = true
    generate_output = 'strain_xx strain_yy strain_zz'
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = temperature_load
  [../]
[]

[BCs]
  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./small_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./thermal_expansion_strain1]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 298
    thermal_expansion_coeff = 1.0e-5
    temperature = temp
    eigenstrain_name = eigenstrain1
  [../]
  [./thermal_expansion_strain2]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 298
    thermal_expansion_coeff = 0.3e-5
    temperature = temp
    eigenstrain_name = eigenstrain2
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.075
  dt = 0.0125
  dtmin = 0.0001
[]

[Outputs]
  exodus = true
  checkpoint = true
[]

[Postprocessors]
  [./strain_xx]
    type = ElementAverageValue
    variable = strain_xx
    block = 0
  [../]
  [./strain_yy]
    type = ElementAverageValue
    variable = strain_yy
    block = 0
  [../]
  [./strain_zz]
    type = ElementAverageValue
    variable = strain_zz
    block = 0
  [../]
  [./temperature]
    type = AverageNodalVariableValue
    variable = temp
    block = 0
  [../]
[]
