[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [./soln]
    type = SolutionRasterizer
    system_variables = 'c'
    mesh = diffuse_out.e
    execute_on = timestep_begin

    variable = c
    xyz_input = in.xyz
    xyz_output = out.xyz

    # raster_mode = MAP
    raster_mode = FILTER
    threshold = 0.5
  [../]
[]
