#This is a simple 1D test of the volumetric heat source with material properties
#of a representative ceramic material.  A bar is uniformly heated, and a temperature
#boundary condition is applied to the left side of the bar.

#Important properties of problem:
#Length: 0.01 m
#Thermal conductivity = 3.0 W/(mK)
#Specific heat = 300.0 J/K
#density = 10431.0 kg/m^3
#Prescribed temperature on left side: 600 K

#When it has reached steady state, the temperature as a function of position is:
# T = -q/(2*k) (x^2 - 2*x*length) + 600
# or
# T = -6.3333e+7 * (x^2 - 0.02*x) + 600
# on left side: T=600, on right side, T=6933.3

[GlobalParams]
  temp = temp
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = bar.e
[]

[Functions]
  [./volumetric_heat]
     type = ParsedFunction
     value = 3.8e+8
  [../]
[]

[Variables]
  [./temp]
    initial_condition = 300.0
  [../]
[]

[AuxVariables]
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
  [./heatsource]
    type = HeatSource
    block = 1
    function = volumetric_heat
    variable = temp
  [../]
[]

[BCs]
  [./lefttemp]
    type = PresetBC
    boundary = 1
    variable = temp
    value = 600
  [../]
[]

[Materials]
  [./heatcond]
    type = HeatConductionMaterial
    block = 1
    thermal_conductivity = 3.0
    specific_heat = 300.0
  [../]
  [./density]
    type = Density
    block = 1
    density = 10431.0
  [../]
[]

[Executioner]
  type = Transient


  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'


  line_search = 'none'


  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

# controls for nonlinear iterations
  nl_max_its = 15
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 10.0
  end_time = 500.0
  num_steps = 5000
[]

[Outputs]
  file_base = heat_source_bar_out
  exodus = true
[]
