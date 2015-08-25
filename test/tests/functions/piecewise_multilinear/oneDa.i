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


[Functions]
# The result (which is unity) that all the functions should yield
  [./answer_fcn]
    type = ConstantFunction
    value = 1
  [../]

# Function that is 1 for all x>=0, due to data only being defined on x<0
  [./end1_fcn]
    type = PiecewiseMultilinear
    data_file = end1.txt
  [../]

# Function that is 1 for all x>=0, due to data only being defined on x<=0
  [./end2_fcn]
    type = PiecewiseMultilinear
    data_file = end2.txt
  [../]

# Function that is 1 for all x<=2, due to data only being defined on x>2
  [./end3_fcn]
    type = PiecewiseMultilinear
    data_file = end3.txt
  [../]

# Function that is 1 for all x<=2, due to data only being defined on x>=2
  [./end4_fcn]
    type = PiecewiseMultilinear
    data_file = end4.txt
  [../]

# Function that is 1 for all x, due to only one point being defined on X at x=2
  [./one_pt1_fcn]
    type = PiecewiseMultilinear
    data_file = one_pt1.txt
  [../]

# Function that is 1 for all x, due to only one point being defined on X at x=1
  [./one_pt2_fcn]
    type = PiecewiseMultilinear
    data_file = one_pt2.txt
  [../]

# Function that is 1 for all x, due to only one point being defined on X at x=-1
  [./one_pt3_fcn]
    type = PiecewiseMultilinear
    data_file = one_pt3.txt
  [../]

# Function that is 1 for all x, and data is defined on Y axis only
  [./other_axis1_fcn]
    type = PiecewiseMultilinear
    data_file = other_axis1.txt
  [../]

# Function that is 1 for all x, and data is defined on T axis only for t>=1
  [./other_axis2_fcn]
    type = PiecewiseMultilinear
    data_file = other_axis2.txt
  [../]

# Function that is 1 for all x, and data that is unity and defined on T axis for -1<=t<=1
  [./other_axis3_fcn]
    type = PiecewiseMultilinear
    data_file = other_axis3.txt
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
