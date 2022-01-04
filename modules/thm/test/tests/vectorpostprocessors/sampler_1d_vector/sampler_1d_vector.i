# Tests the Sampler1DVector vector post-processor, which samples
# a component of a vector-valued material on a block of a 1-D mesh.

[Mesh]
  type = GeneratedMesh
  xmax = 10
  dim = 1
  nx = 5
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Materials]
  [mat]
    type = VectorPropertyTestMaterial
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [test_property_vpp]
    type = Sampler1DVector
    block = 0
    property = test_property
    index = 1
    sort_by = x
  []
[]

[Outputs]
  [out]
    type = CSV
    file_base = out
    execute_vector_postprocessors_on = timestep_end
    show = 'test_property_vpp'
  []
[]
