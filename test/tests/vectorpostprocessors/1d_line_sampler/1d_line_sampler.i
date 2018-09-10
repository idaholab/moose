# Tests the ability of a line sampler to correctly sample a coincident line. In
# 1-D, it was found that sometimes only the first few elements would be found,
# due to floating point precision error in equality tests for the points. This
# test uses a mesh configuration for which this has occurred and ensures that
# the output CSV file contains all points for the LineMaterialRealSampler vector
# postprocessor.

my_xmax = 1.2

[Mesh]
  type = GeneratedMesh
  parallel_type = replicated # Until RayTracing.C is fixed
  dim = 1
  nx = 10
  xmin = 0
  xmax = ${my_xmax}
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

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Materials]
  [./my_mat]
    type = GenericConstantMaterial
    prop_names = 'my_prop'
    prop_values = 5
  [../]
[]

[VectorPostprocessors]
  [./my_vpp]
    type = LineMaterialRealSampler
    property = my_prop
    start = '0 0 0'
    end = '${my_xmax} 0 0'
    sort_by = x
  [../]
[]

[Outputs]
  [./out]
    type = CSV
    execute_vector_postprocessors_on = 'timestep_end'
    show = 'my_vpp'
    precision = 5
  [../]
[]
