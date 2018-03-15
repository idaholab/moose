# calculates the integral of various functions over
# boundaries of the mesh.  See [Postprocessors] for
# a description of the functions
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmin = -1
  xmax = 1
  ymin = -2
  ymax = 2
  zmin = 0
  zmax = 6
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[ICs]
  [./u]
    type = ConstantIC
    variable = u
    value = 0
  [../]
[]

[Postprocessors]
  [./zmin]
    # no function is provided, so it should default to 1
    # yielding postprocessor = 8
    type = FunctionSideIntegral
    boundary = back
  [../]
  [./zmax]
    # result should be -6*area_of_zmax_sideset = -48
    type = FunctionSideIntegral
    boundary = front
    function = '-z'
  [../]
  [./ymin]
    # since the integrand is odd in x, the result should be zero
    type = FunctionSideIntegral
    boundary = bottom
    function = 'x*pow(z,4)'
  [../]
  [./ymax]
    # result should be 24
    type = FunctionSideIntegral
    boundary = top
    function = 'y*(1+x)*(z-2)'
  [../]
  [./xmin_and_xmax]
    # here the integral is over two sidesets
    # result should be 432
    type = FunctionSideIntegral
    boundary = 'left right'
    function = '(3+x)*z'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = function_sideintegral
  [./csv]
    type = CSV
  [../]
[]
