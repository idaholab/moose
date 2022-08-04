# PiecewiseMultilinear function tests in 1D
# See [Functions] block for a description of the tests
# All tests yield variable = 1 everywhere, so they are compared using postprocessors

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 2
  nx = 10
[]

[Variables]
  [./dummy]
  [../]
[]

[Kernels]
  [./dummy_u]
    type = TimeDerivative
    variable = dummy
  [../]
[]


[AuxVariables]
  [./end1_var]
  [../]

  [./end2_var]
  [../]

  [./end3_var]
  [../]

  [./end4_var]
  [../]

  [./one_pt1_var]
  [../]

  [./one_pt2_var]
  [../]

  [./one_pt3_var]
  [../]

  [./other_axis1_var]
  [../]

  [./other_axis2_var]
  [../]

  [./other_axis3_var]
  [../]
[]

[AuxKernels]
  [./end1_auxK]
    type = FunctionAux
    variable = end1_var
    function = end1_fcn
  [../]

  [./end2_auxK]
    type = FunctionAux
    variable = end2_var
    function = end2_fcn
  [../]

  [./end3_auxK]
    type = FunctionAux
    variable = end3_var
    function = end3_fcn
  [../]

  [./end4_auxK]
    type = FunctionAux
    variable = end4_var
    function = end4_fcn
  [../]

  [./one_pt1_auxK]
    type = FunctionAux
    variable = one_pt1_var
    function = one_pt1_fcn
  [../]

  [./one_pt2_auxK]
    type = FunctionAux
    variable = one_pt2_var
    function = one_pt2_fcn
  [../]

  [./one_pt3_auxK]
    type = FunctionAux
    variable = one_pt3_var
    function = one_pt3_fcn
  [../]

  [./other_axis1_auxK]
    type = FunctionAux
    variable = other_axis1_var
    function = other_axis1_fcn
  [../]

  [./other_axis2_auxK]
    type = FunctionAux
    variable = other_axis2_var
    function = other_axis2_fcn
  [../]

  [./other_axis3_auxK]
    type = FunctionAux
    variable = other_axis3_var
    function = other_axis3_fcn
  [../]
[]

[Reporters]
  [end1]
    type = GriddedDataReporter
    data_file = 'end1.txt'
    outputs = none
  []
  [end2]
    type = GriddedDataReporter
    data_file = 'end2.txt'
    outputs = none
  []
  [end3]
    type = GriddedDataReporter
    data_file = 'end3.txt'
    outputs = none
  []
  [end4]
    type = GriddedDataReporter
    data_file = 'end4.txt'
    outputs = none
  []
  [one_pt1]
    type = GriddedDataReporter
    data_file = 'one_pt1.txt'
    outputs = none
  []
  [one_pt2]
    type = GriddedDataReporter
    data_file = 'one_pt2.txt'
    outputs = none
  []
  [one_pt3]
    type = GriddedDataReporter
    data_file = 'one_pt3.txt'
    outputs = none
  []
  [other_axis1]
    type = GriddedDataReporter
    data_file = 'other_axis1.txt'
    outputs = none
  []
  [other_axis2]
    type = GriddedDataReporter
    data_file = 'other_axis2.txt'
    outputs = none
  []
  [other_axis3]
    type = GriddedDataReporter
    data_file = 'other_axis3.txt'
    outputs = none
  []
[]

[Functions]
# The result (which is unity) that all the functions should yield
  [./answer_fcn]
    type = ConstantFunction
    value = 1
  [../]

# Function that is 1 for all x>=0, due to data only being defined on x<0
  [./end1_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'end1/parameter'
    grid_name = 'end1/grid'
    axes_name = 'end1/axes'
    step_name = 'end1/step'
    dim_name = 'end1/dim'
  [../]

# Function that is 1 for all x>=0, due to data only being defined on x<=0
  [./end2_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'end2/parameter'
    grid_name = 'end2/grid'
    axes_name = 'end2/axes'
    step_name = 'end2/step'
    dim_name = 'end2/dim'
  [../]

# Function that is 1 for all x<=2, due to data only being defined on x>2
  [./end3_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'end3/parameter'
    grid_name = 'end3/grid'
    axes_name = 'end3/axes'
    step_name = 'end3/step'
    dim_name = 'end3/dim'
  [../]

# Function that is 1 for all x<=2, due to data only being defined on x>=2
  [./end4_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'end4/parameter'
    grid_name = 'end4/grid'
    axes_name = 'end4/axes'
    step_name = 'end4/step'
    dim_name = 'end4/dim'
  [../]

# Function that is 1 for all x, due to only one point being defined on X at x=2
  [./one_pt1_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'one_pt1/parameter'
    grid_name = 'one_pt1/grid'
    axes_name = 'one_pt1/axes'
    step_name = 'one_pt1/step'
    dim_name = 'one_pt1/dim'
  [../]

# Function that is 1 for all x, due to only one point being defined on X at x=1
  [./one_pt2_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'one_pt2/parameter'
    grid_name = 'one_pt2/grid'
    axes_name = 'one_pt2/axes'
    step_name = 'one_pt2/step'
    dim_name = 'one_pt2/dim'
  [../]

# Function that is 1 for all x, due to only one point being defined on X at x=-1
  [./one_pt3_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'one_pt3/parameter'
    grid_name = 'one_pt3/grid'
    axes_name = 'one_pt3/axes'
    step_name = 'one_pt3/step'
    dim_name = 'one_pt3/dim'
  [../]

# Function that is 1 for all x, and data is defined on Y axis only
  [./other_axis1_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'other_axis1/parameter'
    grid_name = 'other_axis1/grid'
    axes_name = 'other_axis1/axes'
    step_name = 'other_axis1/step'
    dim_name = 'other_axis1/dim'
  [../]

# Function that is 1 for all x, and data is defined on T axis only for t>=1
  [./other_axis2_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'other_axis2/parameter'
    grid_name = 'other_axis2/grid'
    axes_name = 'other_axis2/axes'
    step_name = 'other_axis2/step'
    dim_name = 'other_axis2/dim'
  [../]

# Function that is 1 for all x, and data that is unity and defined on T axis for -1<=t<=1
  [./other_axis3_fcn]
    type = PiecewiseMultilinearFromReporter
    values_name = 'other_axis3/parameter'
    grid_name = 'other_axis3/grid'
    axes_name = 'other_axis3/axes'
    step_name = 'other_axis3/step'
    dim_name = 'other_axis3/dim'
  [../]
[]

[Postprocessors]
  [./end1_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = end1_var
  [../]
  [./end2_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = end2_var
  [../]
  [./end3_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = end3_var
  [../]
  [./one_pt1_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = one_pt1_var
  [../]
  [./one_pt2_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = one_pt2_var
  [../]
  [./one_pt3_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = one_pt3_var
  [../]
  [./other_axis1_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = other_axis1_var
  [../]
  [./other_axis2_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = other_axis2_var
  [../]
  [./other_axis3_pp]
    type = NodalL2Error
    function = answer_fcn
    variable = other_axis3_var
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = oneDa
  hide = dummy
  exodus = false
  csv = true
[]
