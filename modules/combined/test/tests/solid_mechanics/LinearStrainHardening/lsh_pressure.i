#
# This test exercises the linear strain hardening material.  The mesh is
# taken from the patch test (7 elements, 1 on the interior).  There are
# symmetry bcs on three faces with a pressure load on another face.
#
# Young's modulus    = 2.4e5
# Yield stress       = 2.4e2
# Hardening constant = 1600
#
# The pressure reaches 2.4e2 at time 1 and 2.6e2 at time 2.  Thus, at
# time 1, the stress is at the yield stress.  2.4e2/2.4e5=0.001 (the
# strain at time 1).  The increase in stress from time 1 to time 2 is
# 20.  20/1600=0.0125 (the plastic strain).  The elastic strain at
# time 2 is 260/2.4e5=0.00108333.  The total strain at time 2 is
# 0.01358333.
#

[Mesh]
  file = lsh_pressure.e

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

  [./stress_yy]
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

  [./plastic_strain_mag]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]


[Functions]
  [./top_pull]
    type = PiecewiseLinear
    x = '0  1      2'
    y = '0 -2.4e2 -2.6e2'
  [../]

  [./dts]
    type = PiecewiseLinear
    x = '0    0.8 1   1.8'
    y = '0.8  0.2 0.8 0.2'
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

  [./strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_yy
    index = 1
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

  [./plastic_strain_mag]
    type = MaterialRealAux
    property = effective_plastic_strain
    variable = plastic_strain_mag
  [../]

 []


[BCs]

  [./Pressure]
    [./internal_pressure]
      boundary = 11
      function = top_pull
      disp_x = disp_x
      disp_y = disp_y
      disp_z = disp_z
    [../]
  [../]

  [./x]
    type = DirichletBC
    variable = disp_x
    boundary = 10
    value = 0.0
  [../]

  [./y]
    type = DirichletBC
    variable = disp_y
    boundary = 9
    value = 0.0
  [../]

  [./z]
    type = DirichletBC
    variable = disp_z
    boundary = 14
    value = 0.0
  [../]

[]

[Materials]
  [./constant]
    type = LinearStrainHardening
    block = '1 2 3 4 5 6 7'
    youngs_modulus = 2.4e5
    poissons_ratio = 0.3
    yield_stress = 2.4e2
    hardening_constant = 1600
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  line_search = 'none'


  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  start_time = 0.0
  end_time = 2
  dt = 1.5e-3

  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]



[Outputs]
  exodus = true
[]
