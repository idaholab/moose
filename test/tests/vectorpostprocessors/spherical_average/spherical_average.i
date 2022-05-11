[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmin = -5
  xmax = 5
  ymin = -5
  ymax = 5
  zmin = -5
  zmax = 5
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = sin(x*7.4+z*4.1)+cos(y*3.8+x*8.7)+sin(z*9.1+y*2.6)
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = c
  [../]
  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[VectorPostprocessors]
  [./average]
    type = SphericalAverage
    variable = c
    radius = 5
    bin_number = 10
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  solve_type = PJFNK
[]

[Outputs]
  execute_on = 'initial timestep_end'
  csv = true
[]
