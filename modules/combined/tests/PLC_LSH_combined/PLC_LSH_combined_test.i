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
  active = 'disp_x disp_y disp_z temp'

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

  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_yy]
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
    x = '  0   1   1.5'
    y = '-20 -40   -20'
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
    type = HeatConductionImplicitEuler
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

  [./elastic_strain_yy]
    type = MaterialTensorAux
    variable = elastic_strain_yy
    tensor = elastic_strain
    index = 1
  [../]

  [./plastic_strain_yy]
    type = MaterialTensorAux
    variable = plastic_strain_yy
    tensor = plastic_strain
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
    type = Pressure
    variable = disp_y
    component = 1
    boundary = 5
    factor = 1
    function = top_pull
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
    youngs_modulus = 1e3
    poissons_ratio = .3
    coefficient = 0.5e-7
    n_exponent = 5
    m_exponent = -0.5
    activation_energy = 0
    relative_tolerance = 1.e-5
    stress_tolerance = 1e-5
    max_its = 30
    hardening_constant = 100
    yield_stress = 20
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    temp = temp
    formulation = nonlinear3D
  #  formulation = linear
    output_iteration_info = false
  [../]

 [./thermal]
    type = HeatConductionMaterial
    block = 1
    density = 1.0
    specific_heat = 1.0
    thermal_conductivity = 100.
  [../]
[]

[Executioner]
#  type = SolutionTimeAdaptive
  type = Transient

  petsc_options = '-snes_mf_operator -ksp_monitor -snes_ksp'
  petsc_options_iname = '-snes_type -snes_ls -ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = 'ls         basic    101           asm      lu'

  l_max_its = 20
  nl_max_its = 6
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10
  l_tol = 1e-5
  start_time = 0.0
  end_time = 1.4999999999
  dt = 0.001
  time_t  = '0        0.5    1.0    1.5'
  time_dt = '0.015  0.015  0.005  0.005'
 []

[Postprocessors]
  [./timestep]
    type = PrintDT
  [../]
[]

[Output]
  file_base = out
  interval = 1
  output_initial = true
  elemental_as_nodal = true
  exodus = true
  perf_log = true
[]
