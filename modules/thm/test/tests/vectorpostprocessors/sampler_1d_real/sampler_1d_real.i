# Tests the Sampler1DReal vector post-processor, which samples a scalar-valued
# material on a block of a 1-D mesh. This test solves a diffusion problem and
# sets up a constant material to sample.

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
    type = ConstantMaterial
    property_name = test_property
    value = 7
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
    type = Sampler1DReal
    block = 0
    property = test_property
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
