#
# This test evaluates Porosity.C, which is located in elk/src/heat_conduction.
# The purpose of this code is for porosity to be defined as a function of temperature, or a constant.
# Additionally, in the case when porosity is a funciton of temperature an annealing temperature is defined.
# When the annealing temperature is reached, the porosity remains constant (whatever the value of porosity is at the annealing temperature)
# regardless of subsequent values of temperature.
# Porosity.C was included in the heat_conduction directory of elk because
# it will be used in conjuction with other material models that use 
# porosity to define, calculate, or otherwise influence thermal conductivity.
#
# Through boundary conditions, the temperature of a single hex8 element
# is defined with the following function:
 # [./t_func]
 #   type = PiecewiseLinear
 #   x = '0   1   2   3'       time
 #   y = '100 200 200 100'     temperature
 # [../]
#
# The porosity function is defined in the following way:
 # [./p_func]
 #   type = PiecewiseLinear
 #   x = '100  200'         temperature
 #   y = '.01 .005'         porosity
 # [../]
#
# So, as the temperature increases from 100 to 200 the porosity correspondingly
# decreases from 0.01 to 0.005.  The annealing temperature is 200.
# Once the temperature at a qp point reaches 200, the porosity for that qp
# remains at 0.005 for the remainder of the simulation.  A plot of porosity should reflect this behavior.
#
[Mesh]#Comment
  file = simple.e
[] # Mesh

[Functions]
  [./k_func]
    type = PiecewiseLinear
    x = '100 199 200'
    y = '1   1   2'
  [../]

  [./c_func]
    type = PiecewiseLinear
    x = '100    200'
    y = '0.116  0.116'
  [../]

  [./t_func]
    type = PiecewiseLinear
    x = '0   1   2   3'
    y = '100 200 200 100'
  [../]

  [./p_func]
    type = PiecewiseLinear
    x = '100  200'
    y = '.01 .005'
  [../]
[] # Functions

[Variables]

  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
  [../]

[] # Variables

[AuxVariables]

  [./porosity_var]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 0.01
  [../]
[]

[Kernels]

  [./heat_r]
    type = HeatConduction
    variable = temp
  [../]


[] # Kernels

[AuxKernels]

  [./porosity_aux]
    type = MaterialRealAux
    variable = porosity_var
    property = porosity
  [../]

[]

[BCs]

  [./temps_function]
    type = FunctionPresetBC
    variable = temp
    boundary = 1000
    function = t_func
  [../]

  [./flux_in]
    type = NeumannBC
    variable = temp
    boundary = 100
    value = 0
  [../]

[] # BCs

[Materials]

  [./heat]
    type = HeatConductionMaterial
    block = 1
    temp = temp
    thermal_conductivity_temperature_function = k_func
    specific_heat_temperature_function = c_func
  [../]

  [./porosity]
    type = Porosity
    block = 1
    temp = temp
    porosity_temperature_function = p_func
    anneal_temp = 200
  [../]

  [./density]
    type = Density
    block = 1
    density = 0.283
  [../]

[] # Materials


[Executioner]

  type = Transient

  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -snes_type -snes_ls -snes_linesearch_type -ksp_gmres_restart'
  petsc_options_value = 'lu       ls         basic    basic                    101'

  l_max_its = 100
  l_tol = 8e-3

  nl_max_its = 15
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 1
  end_time = 3
  num_steps = 3


[] # Executioner

[Output]
  file_base = function
  interval = 1
  output_initial = true
  exodus = true
  perf_log = true
[] # Output
