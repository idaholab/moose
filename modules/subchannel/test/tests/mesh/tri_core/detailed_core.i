# Detailed (3D) counterpart of core.i: a 19-pin assembly in the center surrounded by six 7-pin
# assemblies, on the same lattice, for visualizing core solutions
[Mesh]
  [large]
    type = SCMDetailedTriAssemblyMeshGenerator
    nrings = 3
    n_cells = 2
    flat_to_flat = 0.6
    heated_length = 1.0
    pin_diameter = 0.1
    pitch = 0.13
  []
  [small]
    type = SCMDetailedTriAssemblyMeshGenerator
    nrings = 2
    n_cells = 2
    flat_to_flat = 0.4
    heated_length = 1.0
    pin_diameter = 0.1
    pitch = 0.13
  []
  [core]
    type = SCMTriCoreMeshGenerator
    inputs = 'large small'
    assembly_pitch = 0.52
    pattern = '1 1;
              1 0 1;
               1 1'
  []
[]

# Fields a solution transfer would fill, restricted to the per-assembly blocks
[AuxVariables]
  [T]
    block = 'subchannel_0 subchannel_1 subchannel_2 subchannel_3 subchannel_4 subchannel_5
             subchannel_6'
  []
  [q_prime]
    block = 'fuel_pins_0 fuel_pins_1 fuel_pins_2 fuel_pins_3 fuel_pins_4 fuel_pins_5 fuel_pins_6'
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
