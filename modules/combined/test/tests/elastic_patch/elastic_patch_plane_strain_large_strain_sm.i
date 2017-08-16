# Deprecated: large_strain = true only introduces high order terms in the strain calculation
# but no rotation has been considered in solid mechanics. No such corresponding strain calculator
# in tensor mechanics
#
#
# This problem is adapted from the Abaqus verification manual:
#   "1.5.1 Membrane patch test"
#
# For large strain,
#   e_xx = e_yy = 1e-3 + 0.5*((1e-3)^2+0.25*(1e-3)^2) = 0.001000625
#   e_xy = 0.5*(1e-3 + (1e-3)^2)                      = 0.0005005
#
# If you multiply these strains through the elasticity tensor,
#   you will obtain the following stresses:
#   xx = yy = 1601.0
#   zz      =  800.5
#   xy      =  400.4
#   yz = zx =    0
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = elastic_patch_rz.e
[]

[Functions]
  [./ux]
    type = ParsedFunction
    value = '1e-3*(x+0.5*y)'
  [../]
  [./uy]
    type = ParsedFunction
    value = '1e-3*(y+0.5*x)'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./temp]
    initial_condition = 117.56
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
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
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
  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
  [../]
  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
  [../]
[]

[BCs]
  [./ur]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 10
    function = ux
  [../]
  [./uz]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 10
    function = uy
  [../]

  [./temp]
    type = DirichletBC
    variable = temp
    boundary = 10
    value = 117.56
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1

    disp_x = disp_x
    disp_y = disp_y

    youngs_modulus = 1e6
    poissons_ratio = 0.25

    temp = temp

    formulation = planestrain

    large_strain = true
  [../]

  [./heat]
    type = HeatConductionMaterial
    block = 1

    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
  [../]

  [./density]
    type = Density
    block = 1
    density = 0.283
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  file_base = elastic_patch_plane_strain_large_strain_out
  exodus = true
[]
