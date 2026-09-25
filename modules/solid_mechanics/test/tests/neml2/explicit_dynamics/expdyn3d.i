N = 2

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
  nx = ${N}
  ny = ${N}
  nz = ${N}
  parallel_type = DISTRIBUTED
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Functions]
  [forcing_fn]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2    0.3  0.4    0.5  0.6'
    y = '0.0 0.0 0.0025 0.01 0.0175 0.02 0.02'
  []
[]

[Kernels]
  [mx]
    type = MassMatrix
    density = density
    variable = disp_x
    matrix_tags = 'mass'
  []
  [my]
    type = MassMatrix
    density = density
    variable = disp_y
    matrix_tags = 'mass'
  []
  [mz]
    type = MassMatrix
    density = density
    variable = disp_z
    matrix_tags = 'mass'
  []
[]

[Materials]
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = 1
  []
[]

[BCs]
  [pull_z]
    type = ExplicitFunctionDirichletBC
    variable = disp_z
    boundary = 'front'
    function = forcing_fn
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0
  []
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  []
[]

[Postprocessors]
  [disp_z]
    type = ElementAverageValue
    variable = disp_z
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  [exodus]
    type = Exodus
    execute_on = 'INITIAL FINAL'
  []
  [console]
    type = Console
    execute_postprocessors_on = 'INITIAL FINAL'
  []
[]
