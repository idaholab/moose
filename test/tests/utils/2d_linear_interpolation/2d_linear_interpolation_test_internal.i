# Test description - view this file in emacs and adjust the window size to view the file as it was created.
#
# This problem tests the MOOSE function PiecewiseBilinear and the MOOSE utility BilinearInterpolation, which are
# used to solve 2D linear interpolation problems.
#
# The problem is one element with node coordinate locations in x,y,z space as indicated in the ASCII art:
#
#
#                    ^
#                    |
#                    z
#                    |
#
#                    (1,1,2)        (1,2,2)
#                    *--------------*
#                  / |            / |
#                /   |  (2,2,2) /   |
#       (2,1,2) *--------------*    |
#               |    |         |    |
#               |    *---------|----* (1,2,1)  --y-->
#               |  / (1,1,1)   |  /
#               |/             |/
#               *--------------*
#              / (2,1,1)       (2,2,1)
#            /
#          x
#        /
#      |_
#
#  problem time ...0...1...2
#
#
# There are four variables and four functions of the same name, u,v,w, and A.  The diffusion equation is solved
# for each of these variables with a boundary condition of type FunctionDirchletBC applied to a boundary
# (i.e. node set) that includes every node in the element.  Each boundary condition uses a function of type
# PiecewiseBilinear that gets its value from the specified x, y, and z values.
#
# fred is a matrix of data whose first row and first column are vectors that can refer to either spacial positions
# corresponding to an axis or values of time.  The remaining data are values of fred for a given row and column pair.
#
#
# Visualize fred like this:
#
#                          0 1 3  where fred is a csv file that actually looks like this    0,1,3
#                        0 0 0 0                                                            0,0,0,0
#                        1 0 1 3                                                            1,0,1,3
#                        3 0 5 7                                                            3,0,5,7
#
#  Another way to think of fred is:
#
#                                   |0 1 3| - These values can be spacial positions corresponding to
#                                             axis= 0,1, or 2, or time
#
#
#                           |0|     |0 0 0|
#     These values can be - |1|     |0 1 3| - values of fred corresponding to row-column pairs
#     time or spacial       |3|     |0 5 7|
#     positions corresponding
#     to axis= 0,1, or 2
#
#
# The parameters and possible values for the function PiecewiseBilinear are:
#
# x = '0 1 3'
# y = '0 1 3'
# z = '0 0 0 0 1 3 0 5 7'
# axis = 0, 1, or 2
# xaxis = 0, 1, or 2
# yaxis = 0, 1, or 2
# radial = true or false (false is default)
#
# where 0, 1, or 2 refer to the x, y, or z axis.
#
# If the parameter axis is defined, then the first row of fred are spacial position and the first column
# of fred are the values of time.
#
# If the parameter xaxis is defined, then the first row of fred are spacial positions and the first column
# of fred are the values of time ... just like defining the parameter axis.
#
# If the parameter yaxis is defined, then the first row of fred are time values and the first column of fred
# are spacial positions.
#
# If parameters axis AND EITHER xaxis or yaxis are defined together you'll get a moose error.
# i.e.
# axis = 0
# xaxis = 1
# results in an error.  So, if you use the parameter axis, don't use xaxis or yaxis.
#
# If parameters xaxis and yaxis are defined (and radial is false), then the first row of fred are spacial positions corresponding to xaxis value,
# and the first column are spacial positions corresponding to the yaxis value.
#
# If xaxis and yaxis are defined and radial is true, the first row of fred contains values
# corresponding to the radius calculated from the coordinates of each point.  Note that
# the definition of xaxis and yaxis define the "plane" of the radius.  For example,
# xaxis = 0 and yaxis = 1 means that x and y components of the point are use to
# calculate the radius.  xaxis = 1 and yaxis = 2 means that x and z components are used.
# The first column is for time in this case.  xaxis and yaxis have to be specified and
# radial = true for this to work, otherwise a MOOSE error will result.
# This was developed so that an axisymmetric function could be defined for a 3D mesh.
#
[Mesh]
  file = cube.e
  # This problem only has 1 element, so using DistributedMesh in parallel
  # isn't really an option, and we don't care that much about DistributedMesh
  # in serial.
  parallel_type = replicated
[]

[Variables]

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
  [./A]
    order = FIRST
    family = LAGRANGE
  [../]
  [./scaled_u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./R]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]


  [./u]
    type = PiecewiseBilinear
    x = '0 1 3'
    y = '0 1 3'
    z = '0 0 0 0 1 3 0 5 7'
    axis = 0
  [../]
#
# Example 1 - variable u
#
# In this example, the first variable is u and the parameter axis is given the value 0.  For such a case, the first
# row of fred refers to nodal x-coordinate values and the first column of fred (after the first row) refers to the
# times 0, 1, and 3.
#
# So, at time = 0, the value of u at each node is 0, because that's the value of fred for all x-coordinate values at time=0.
#
# At time = 1, the value of u at nodes with x-coordinate = 1 is 1.
#            , the value of u at nodes with x-coordinate = 2 is 2.
#
# You can check this value with your own 2D linear interpolation calculation.  Go ahead and check all the examples!
#
# At time = 2, the value of u at nodes with x-coordinate = 1 is 3.
#            , the value of u at nodes with x-coordinate = 2 is 4.
#
  [./v]
    type = PiecewiseBilinear
    x = '0 1 3'
    y = '0 1 3'
    z = '0 0 0 0 1 3 0 5 7'
    xaxis = 1
  [../]
#
# Example 2 - variable v
#
# In this example, the variable is v and the parameter xaxis is given the value 1.  For such a case, the first
# row of fred refers to nodal y-coordinate values and the first column of fred (after the first row) refers to the
# times 0, 1, and 3.
#
# At time = 0, the value of v at each node is 0, because that's the value of fred for all y-coordinate values at time=0.
#
# At time = 1, the value of v at nodes with y-coordinate = 1 is 1.
#            , the value of v at nodes with y-coordinate = 2 is 2.
#
# At time = 2, the value of v at nodes with y-coordinate = 1 is 3.
#            , the value of v at nodes with y-coordinate = 2 is 4.
#
  [./w]
    type = PiecewiseBilinear
    x = '0 1 3'
    y = '0 1 3'
    z = '0 0 0 0 1 3 0 5 7'
    yaxis = 2
  [../]
#
# Example 3 - variable w
#
# In this example, the variable is w and the parameter yaxis is given the value 2.  For such a case, the first
# row of fred refers to times 0, 1, and 3.  The first column of fred (after the first row) refers to the nodal
# z-coordinate values.
#
# At time = 0, the value of w at each node is 0, because that's the value of fred for all z-coordinate values at time=0.
#
# At time = 1, the value of w at nodes with z-coordinate = 1 is 1.
#            , the value of w at nodes with z-coordinate = 2 is 3.
#
# At time = 2, the value of w at nodes with z-coordinate = 1 is 2.
#            , the value of w at nodes with z-coordinate = 2 is 4.
#
  [./A]
    type = PiecewiseBilinear
    x = '0 1 3'
    y = '0 1 3'
    z = '0 0 0 0 1 3 0 5 7'
    xaxis = 0
    yaxis = 1
  [../]
#
# Example 4 - variable A
#
# In this example, the variable is A and the parameters xaxis AND yaxis BOTH defined and given the values 0 and 1 respectivley.
# For such a case, the first row of fred refers to nodal x-coordinate values.
# The first column refers to nodal y-coordinate values.
#
# In this example the values are the same for every time (except time=0 where the values are undefined)
#
# For nodal coordinates with x=1, y=1 A = 1
#                            x=2, y=1 A = 2
#                            x=1, y=2 A = 3
#                            x=2, y=2 A = 4
#
# You can use this 2D linear interpolation function for anything (BC, Kernel, AuxKernel, Material) that has
# a function as one of its parameters.  For example, this can be used to describe the fission peaking factors
# that vary in time and along the length of a fuel rod, or a fission rate distribution in metal fuel that varies
# as a function of x and y postion, but is constant in time.
#
#
  [./scaled_u]
    type = PiecewiseBilinear
    x = '0 1 3'
    y = '0 1 3'
    z = '0 0 0 0 1 3 0 5 7'
    axis = 0
    scale_factor = 2
  [../]
#
# Example 5 - variable scaled_u.  This is just a scaled version of Example 1 to see if the scale_factor works
#
#
#
  [./R]
    type = PiecewiseBilinear
    x = '0 1 3'
    y = '0 1 3'
    z = '0 0 0 0 1 3 0 5 7'
    xaxis = 0
    yaxis = 1
    radial = true
  [../]
#
# Example 6 - variable R
#
# In this example, the variable is R and the parameters xaxis and yaxis are defined and
# given the values 0 and 1 respectivley.  The parameter radial is also defined and given
# the value true.  In this case, the x and y components of each point are used to
# calculate a radius.  This radius is used in the call to BilinearInterpolation.
# In fred.csv, the first row are the radius values.  The first column is time.
#
# At time = 1, the value of R at nodes with coordinates (x = 1, y = 1, or r = 1.414) is 1.414.
#            , the value of R at nodes with coordinates (x = 1, y = 2, or r = 2.236) is 2.236.
#            , the value of R at nodes with coordinates (x = 2, y = 2, or r = 2.828) is 2.828.
#
# At time = 2, the value of R at nodes with coordinates (x = 1, y = 1, or r = 1.414) is 3.414.
#            , the value of R at nodes with coordinates (x = 1, y = 2, or r = 2.236) is 4.236.
#            , the value of R at nodes with coordinates (x = 2, y = 2, or r = 2.828) is 4.828.
#
# Note that the case of x = 2, y = 1 gives the same result as x = 1, y=2.
#
#
[] # End Functions

[Kernels]

  [./diffu]
    type = Diffusion
    variable = u
  [../]
  [./diffv]
    type = Diffusion
    variable = v
  [../]
  [./diffw]
    type = Diffusion
    variable = w
  [../]
  [./diffA]
    type = Diffusion
    variable = A
  [../]
  [./diff_scaled_u]
    type = Diffusion
    variable = scaled_u
  [../]
  [./diffR]
    type = Diffusion
    variable = R
  [../]
[]

[BCs]

  [./u]
    type = FunctionDirichletBC
    variable = u
    boundary = '1'
    function = u
  [../]
  [./v]
    type = FunctionDirichletBC
    variable = v
    boundary = '1'
    function = v
  [../]
  [./w]
    type = FunctionDirichletBC
    variable = w
    boundary = '1'
    function = w
  [../]
  [./A]
    type = FunctionDirichletBC
    variable = A
    boundary = '1'
    function = A
  [../]
  [./scaled_u]
    type = FunctionDirichletBC
    variable = scaled_u
    boundary = '1'
    function = scaled_u
  [../]
  [./R]
    type = FunctionDirichletBC
    variable = R
    boundary = '1'
    function = R
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 2
  nl_rel_tol = 1e-12
[]

[Outputs]
  file_base = out
  exodus = true
[]
