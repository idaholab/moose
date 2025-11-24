[Mesh]
  [sphere_cell_univ_fill]
    type = NestedLatticeCellUniverseMeshGenerator
    inner_radius = 3
    outer_radius = 4
    lattice_radius = 10
  []
  [copy_1]
    type = TestOneToManyDependencyMeshGenerator
    input = sphere_cell_univ_fill
    copy_id = 1
  []
  [copy_2]
    type = TestOneToManyDependencyMeshGenerator
    input = sphere_cell_univ_fill
    copy_id = 2
  []
  [copy_combiner]
    type = TestOneToManyDependencyCombinerGenerator
    inputs = 'copy_1 copy_2'
  []
[]
