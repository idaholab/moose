#
# 1x1x1 unit cube with constant displacement on top face
#
# This problem was taken from "Finite element three-dimensional elastic-plastic
#    creep analysis" by A. Levy, Eng. Struct., 1981, Vol. 3, January, pp. 9-16.
#
# The problem is a one-dimensional creep analysis.  The top face is displaced 0.01
#    units and held there.  The stress relaxes in time according to the creep law.
#
# The analytic solution to this problem is (contrary to what is shown in the paper):
#
#                /       (E*ef)^3     \^(1/3)
#    stress_yy = |--------------------|
#                \ 3*a*E^4*ef^3*t + 1 /
#
#    where E  = 2.8e7 (Young's modulus)
#          a  = 3e-26 (creep coefficient)
#          ef = 0.01  (displacement)
#          t  =       (time)
#
# The solution computed is very close to the exact solution.  This test is not a
#     correct representation of the problem since it does not set the initial
#     condition for the displacement and the stress.  Currently (1 March 2011),
#     no capability exists for setting the initial condition of stress.  Until
#     that is available, some error will be inherent in the solution.
#
[Mesh]
  file = 1x1x1_cube.e
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

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1000.0
  [../]
[]

[AuxVariables]

  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./top_pull]
    type = PiecewiseLinear
    x = '0 20'
    y = '0 1'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
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
  [./stress_yy]
    type = MaterialTensorAux
    variable = stress_yy
    tensor = stress
    index = 1
  [../]

  [./creep_strain_yy]
    type = MaterialTensorAux
    variable = creep_strain_yy
    tensor = creep_strain
    index = 1
  [../]
[]


[BCs]
  [./u_top_pull]
    type = PresetBC
    variable = disp_y
    boundary = 5
    value = 0.01
  [../]
  [./u_bottom_fix]
    type = PresetBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]
  [./u_yz_fix]
    type = PresetBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]
  [./u_xy_fix]
    type = PresetBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

  [./temp_top_fix]
    type = PresetBC
    variable = temp
    boundary = 5
    value = 1000.0
  [../]
  [./temp_bottom_fix]
    type = PresetBC
    variable = temp
    boundary = 3
    value = 1000.0
  [../]
[]

[Materials]
  [./creep_plas]
    type = PLC_LSH
    block = 1
    youngs_modulus = 2.8e7
    poissons_ratio = .3
    coefficient = 3.0e-26
    n_exponent = 4
    activation_energy = 0
    relative_tolerance = 1.e-5
    max_its = 100
    hardening_constant = 1
    yield_stress = 1e30
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    temp = temp
    output_iteration_info = false
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
    density = 1
  [../]
[]

[Executioner]
#  type = SolutionTimeAdaptive
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101           asm      lu'


  line_search = 'none'


  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-5
  l_tol = 1e-5
  start_time = 0.0
#  end_time = 2160000
  end_time = 21600
#  num_steps = 50
#  dt = 1.e-3
#  dtmax = 1.e-4
  dt = 1e-2
  [./TimeStepper]
    type = FunctionDT
    time_dt = '1e-2 1e-1 1e0 1e1 1e2'
    time_t  = '0    7e-1 7e0 7e1 1e2'
  [../]
[]

[Postprocessors]
  [./timestep]
    type = TimestepSize
  [../]
[]

[Outputs]
  exodus = true
[]
