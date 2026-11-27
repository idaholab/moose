# Nineteen-position hexagonal core with six empty positions (-1), built from ducted 7-pin
# assemblies
[TriSubChannelMesh]
  [assembly]
    type = SCMTriAssemblyMeshGenerator
    nrings = 2
    n_cells = 2
    flat_to_flat = 0.4
    heated_length = 1.0
    pin_diameter = 0.1
    pitch = 0.13
    dwire = 0.03
    hwire = 0.3
  []
  [duct]
    type = SCMTriDuctMeshGenerator
    input = assembly
    nrings = 2
    n_cells = 2
    flat_to_flat = 0.4
    heated_length = 1.0
    pitch = 0.13
  []
  [core]
    type = SCMTriCoreMeshGenerator
    inputs = 'duct'
    assembly_pitch = 0.42
    pattern = '-1  0 -1;
              0  0  0  0;
            -1  0  0  0 -1;
              0  0  0  0;
               -1  0 -1'
  []
[]
