# Seven-position hexagonal core of core.i built from assemblies without duct meshes
[TriSubChannelMesh]
  [large]
    type = SCMTriAssemblyMeshGenerator
    nrings = 3
    n_cells = 2
    flat_to_flat = 0.6
    heated_length = 1.0
    pin_diameter = 0.1
    pitch = 0.13
    dwire = 0.03
    hwire = 0.3
  []
  [small]
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
  [core]
    type = SCMTriCoreMeshGenerator
    inputs = 'large small'
    # Neighboring 0.6 m and 0.4 m ducts touch at a 0.5 m pitch; 0.52 m leaves a 0.02 m gap
    assembly_pitch = 0.52
    pattern = '1 1;
              1 0 1;
               1 1'
  []
[]
