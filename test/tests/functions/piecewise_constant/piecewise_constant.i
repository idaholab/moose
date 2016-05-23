# This tests the PiecewiseConstant function.
# There are four variables and four functions: a,b,c, and d.  The diffusion equation is "solved"
# for each of these variables with a boundary condition of type FunctionDirchletBC applied to a boundary
# (i.e. node set) that includes every node in the element, so the solution is the boundary condition defined by the function.
#  Each boundary condition uses a function of type PiecewiseConstant.
#
# The value of the variables should correspond to the function.

[Mesh]
  file = cube.e
  # This problem only has 1 element, so using DistributedMesh in parallel
  # isn't really an option, and we don't care that much about DistributedMesh
  # in serial.
  parallel_type = replicated
[]

[Variables]

  [./aVar]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
  [./bVar]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
  [./cVar]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
  [./dVar]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
[]

[Functions]
  [./a]
    type = PiecewiseConstant
    xy_data = '0.5 0.1
               1.0 0.2
               1.5 0.1'
    direction = left
  [../]
  [./b]
    type = PiecewiseConstant
    x = '0.5 1.0 1.5'
    y = '0.1 0.2 0.1'
    direction = right
  [../]
  [./c]
    type = PiecewiseConstant
    data_file = pc.csv
    direction = left
    format = columns
  [../]
  [./d]
    type = PiecewiseConstant
    data_file = pc.csv
    direction = right
    format = columns
  [../]
[]

[Kernels]
  [./diffa]
    type = Diffusion
    variable = aVar
  [../]
  [./diffb]
    type = Diffusion
    variable = bVar
  [../]
  [./diffc]
    type = Diffusion
    variable = cVar
  [../]
  [./diffd]
    type = Diffusion
    variable = dVar
  [../]
[]

[BCs]
  [./a]
    type = FunctionDirichletBC
    variable = aVar
    boundary = '1'
    function = a
  [../]
  [./b]
    type = FunctionDirichletBC
    variable = bVar
    boundary = '1'
    function = b
  [../]
  [./c]
    type = FunctionDirichletBC
    variable = cVar
    boundary = '1'
    function = c
  [../]
  [./d]
    type = FunctionDirichletBC
    variable = dVar
    boundary = '1'
    function = d
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.2
  end_time = 3
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
