[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[MultiApps]
  active = 'sub_1'
  [sub_1]
    type = FullSolveMultiApp
    positions = '0 0 0
                 1 1 1'
    input_files = 'sub.i'
    cli_args = 'Mesh/mesh/type=CartesianMeshGenerator;Mesh/mesh/dim=1;Mesh/mesh/dx="1 2 1";Mesh/mesh/ix="4 3 1"'
  []
  [sub_1_split]
    type = FullSolveMultiApp
    positions = '0 0 0
                 1 1 1'
    input_files = 'sub.i'
    cli_args = 'Mesh/mesh/type=CartesianMeshGenerator;Mesh/mesh/dim=1;Mesh/mesh/dx="1 2 1";Mesh/mesh/ix="4 3 1" Mesh/mesh/type=CartesianMeshGenerator;Mesh/mesh/dim=1;Mesh/mesh/dx="1 2 1";Mesh/mesh/ix="4 3 1"'
  []
  [sub_2]
    type = FullSolveMultiApp
    positions = '0 0 0
                 1 1 1'
    input_files = 'sub.i'
    cli_args = "Mesh/mesh/type=CartesianMeshGenerator;Mesh/mesh/dim=1;Mesh/mesh/dx='1 2 "
               "1';Mesh/mesh/ix='4 3 1'"
  []
  [sub_2_split]
    type = FullSolveMultiApp
    positions = '0 0 0
                 1 1 1'
    input_files = 'sub.i'
    cli_args = "Mesh/mesh/type=CartesianMeshGenerator;Mesh/mesh/dim=1;Mesh/mesh/dx='1 2 "
               "1';Mesh/mesh/ix='4 3 1' "
               "Mesh/mesh/type=CartesianMeshGenerator;Mesh/mesh/dim=1;Mesh/mesh/dx='1 2 "
               "1';Mesh/mesh/ix='4 3 1'"
  []
[]
