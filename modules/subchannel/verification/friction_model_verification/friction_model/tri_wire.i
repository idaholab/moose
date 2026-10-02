# Friction factor sampling for the 61-pin EBR-II XX09 wire-wrapped triangular lattice
G_exp0 = -1.1
# Subchannel indices of an interior, an edge, and a corner subchannel
center = 0
edge = 100
corner = 97

!include friction_factor_sampling.i

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    nrings = 5
    n_cells = 10
    flat_to_flat = 0.0464
    heated_length = 1
    pin_diameter = 0.004419
    pitch = 0.005664
    dwire = 0.001244
    hwire = 0.1524
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  friction_closure = 'Chen'
[]
