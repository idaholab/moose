# Create a mesh that looks like:
# -------------
# | 7 | 6 | 5 |
# -------------
# | 8 | 0 | 4 |
# ------------
# | 1 | 2 | 3 |
# -------------
# Where there is an internal interface around block 0
[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1'
    dy = '1 1 1'
    subdomain_id = '7 6 5
                    8 0 4
                    1 2 3'
  []

  [interface]
    type = SideSetsAroundSubdomainGenerator
    input = mesh
    block = 0
    new_boundary = 'interface'
  []

  uniform_refine = 5
[]

# Here we output all the connections as vectorpostprocessors
[VectorPostprocessors]
  [interface]
    type = SubdomainBoundaryConnectivity
    boundary = interface
    interface_boundary = true
  []
  [interface_parent]
    type = SubdomainBoundaryConnectivity
    boundary = interface
  []
  [block_0]
    type = SubdomainBoundaryConnectivity
    block = 0
  []
  [block_1]
    type = SubdomainBoundaryConnectivity
    block = 1
    interface_boundary = true
  []
  [block_2]
    type = SubdomainBoundaryConnectivity
    block = 2
    interface_boundary = true
  []
  [block_2_parent]
    type = SubdomainBoundaryConnectivity
    block = 2
  []
[]

[Variables]
  [u]
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  exodus = false
  csv = true
[]
