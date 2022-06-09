# This tests the capability of the code to read input files in csv or space separated format.
# There are four variables and four functions of the same name; a,b,c, and d.  The diffusion equation is "solved"
# for each of these variables with a boundary condition of type FunctionDirchletBC applied to a boundary
# (i.e. node set) that includes every node in the element, so the solution is the boundary condition defined by the function.
#  Each boundary condition uses a function of type PiecewiseLinear that gets its value from a file,
# which could be in comma separated or space separated format.  The input file can also contain comments.
#
# The files could have the form
# 0,1,2,3 # time
# 0,4,5,6 # bc value
# for format = row
# or
# 0,0
# 1,4
# 2,5
# 3,6
# for format = column
# Values in files could be separated by white space.  See the .csv and .dat files for format examples.
#
# The value of the variables should correspond to the function.
# At time = 0, the variable = 0, at time = 1, variable = 4 and so on.
[Mesh]
  file = cube.e
  # This problem only has 1 element, so using DistributedMesh in parallel
  # isn't really an option, and we don't care that much about DistributedMesh
  # in serial.
  parallel_type = replicated
[]

[Variables]
  [a]
  []
  [b]
  []
  [c]
  []
  [d]
  []
  [e]
  []
  [f]
  []
[]

[Functions]
  [a]
    type = PiecewiseLinear
    data_file = rows.csv
    format = rows
  []
  [b]
    type = PiecewiseLinear
    data_file = columns.csv
    format = columns
  []
  [c]
    type = PiecewiseLinear
    data_file = rows_space.dat
    format = rows
  []
  [d]
    type = PiecewiseLinear
    data_file = columns_space.dat
    format = columns
  []
  [e_func]
    type = PiecewiseLinear
    data_file = rows_more_data.csv
    format = rows
    xy_in_file_only = false
  []
  [f]
    type = PiecewiseLinear
    data_file = columns_more_data.csv
    format = columns
    xy_in_file_only = false
  []
[]

[Kernels]
  [diffa]
    type = Diffusion
    variable = a
  []
  [diffb]
    type = Diffusion
    variable = b
  []
  [diffc]
    type = Diffusion
    variable = c
  []
  [diffd]
    type = Diffusion
    variable = d
  []
  [diffe]
    type = Diffusion
    variable = e
  []
  [difff]
    type = Diffusion
    variable = f
  []
[]

[BCs]
  [a]
    type = FunctionDirichletBC
    variable = a
    boundary = '1'
    function = a
  []
  [b]
    type = FunctionDirichletBC
    variable = b
    boundary = '1'
    function = b
  []
  [c]
    type = FunctionDirichletBC
    variable = c
    boundary = '1'
    function = c
  []
  [d]
    type = FunctionDirichletBC
    variable = d
    boundary = '1'
    function = d
  []
  [e]
    type = FunctionDirichletBC
    variable = e
    boundary = '1'
    function = e_func
  []
  [f]
    type = FunctionDirichletBC
    variable = f
    boundary = '1'
    function = f
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 3
  nl_rel_tol = 1e-12
[]

[Outputs]
  file_base = out
  exodus = true
[]
