# Detailed (3D) counterpart of core.i, on the same lattice, for visualizing core solutions
[GlobalParams]
  n_cells = 2
  heated_length = 1.0
  pitch = 0.13
  side_gap = 0.05
  pin_diameter = 0.1
[]

[Mesh]
  [big]
    type = SCMDetailedQuadAssemblyMeshGenerator
    nx = 4
    ny = 3
  []
  [small]
    type = SCMDetailedQuadAssemblyMeshGenerator
    nx = 3
    ny = 2
  []
  [core]
    type = SCMQuadCoreMeshGenerator
    inputs = 'big small'
    assembly_pitch = 0.6
    pattern = '0 1  0;
               1 0 -1'
  []
[]

# Fields a solution transfer would fill, restricted to the per-assembly blocks
[AuxVariables]
  [T]
    block = 'subchannel_0 subchannel_1 subchannel_2 subchannel_3 subchannel_4'
  []
  [q_prime]
    block = 'fuel_pins_0 fuel_pins_1 fuel_pins_2 fuel_pins_3 fuel_pins_4'
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [exodus]
    type = Exodus
    output_extra_element_ids = true
  []
[]
