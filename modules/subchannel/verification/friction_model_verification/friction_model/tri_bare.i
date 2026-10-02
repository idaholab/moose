# Friction factor sampling for the 19-pin LBE bare-pin triangular lattice
G_exp0 = -1.6
# Subchannel indices of an interior, an edge, and a corner subchannel
center = 0
edge = 24
corner = 25

!include friction_factor_sampling.i

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    nrings = 3
    n_cells = 10
    flat_to_flat = 0.05319936
    heated_length = 1
    pin_diameter = 0.0082
    pitch = 0.01148
    dwire = 0.0
    hwire = 0.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  friction_closure = 'Chen'
[]
