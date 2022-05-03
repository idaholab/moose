[Mesh]
    [Top_Block]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 10
        ny = 10
        nz = 10
        xmax = 2
        ymax = 2
        zmax = 2
        xmin = 0
        ymin = 0
        zmin = 1
        boundary_name_prefix = 'Upper'
        boundary_id_offset = 10
    []
    [Bottom_Block]
        type = GeneratedMeshGenerator
        dim = 3
        nx = 10
        ny = 10
        nz = 10
        xmax = 2
        ymax = 2
        zmax = 1
        boundary_name_prefix = 'Lower'
    []
    [Combine]
        type = CombinerGenerator
        inputs = 'Top_Block Bottom_Block'
        positions = '0 0 0 0.12 0.12 0'
    []
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

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
[Outputs]
  exodus = true
[]
