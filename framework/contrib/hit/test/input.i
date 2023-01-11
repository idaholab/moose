# This tests the thermal expansion coefficient function using both
# options to specify that function: mean and instantaneous.  There
# two blocks, each containing a single element, and these use the
# two variants of the function.

# In this test, the instantaneous CTE function has a constant value,
# while the mean CTE function is an analytic function designed to
# give the same response.  If \bar{alpha}(T) is the mean CTE function,
# and \alpha(T) is the instantaneous CTE function,

# \bar{\alpha}(T) = 1/(T-Tref) \intA^{T}_{Tsf} \alpha(T) dT

# where Tref is the reference temperature used to define the mean CTE
# function, and Tsf is the stress-free temperature.

# The two models produce very similar results.  There are slight
# differences due to the large deformation treatment.

[Mesh]
  file = 'blocks.e'
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
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 3
    function = 0.02*t
  [../]

  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]

  [./back]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 1
    function = 0.01*t
  [../]
[]

[AuxKernels]
  [./temp]
    type = FunctionAux
    variable = temp
    block = '1 2'
    function = temp_func
  [../]
[]

[Materials]
  [./mean_alpha]
    type = Elastic
    block = 1
    youngs_modulus = 1e6
    poissons_ratio = .3
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    temp = temp
    thermal_expansion_function = cte_func_mean
    stress_free_temperature = 0.0
    thermal_expansion_reference_temperature = 0.5
    thermal_expansion_function_type = mean
  [../]

  [./inst_alpha]
    type = Elastic
    block = 2
    youngs_modulus = 1e6
    poissons_ratio = .3
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    temp = temp
    thermal_expansion_function = cte_func_inst
    stress_free_temperature = 0.0
    thermal_expansion_function_type = instantaneous
  [../]
[]

[Functions]
  [./cte_func_mean]
    type = ParsedFunction
    symbol_names = 'tsf tref scale' #stress free temp, reference temp, scale factor
    symbol_values = '0.0 0.5  1e-4'
    expression = 'scale * (t - tsf) / (t - tref)'
  [../]
  [./cte_func_inst]
    type = PiecewiseLinear
    xy_data = '0 1.0
               2 1.0'
    scale_factor = 1e-4
  [../]

  [./temp_func]
    type = PiecewiseLinear
    xy_data = '0 1
               1 2'
  [../]
[]

[Postprocessors]
  [./disp_1]
    type = NodalMaxValue
    variable = disp_x
    boundary = 101
  [../]

  [./disp_2]
    type = NodalMaxValue
    variable = disp_x
    boundary = 102
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
  end_time = 1.0
  dt = 0.1
[]

[Outputs]
  file_base = const_alpha_out
  exodus = true
  csv = true
[]
