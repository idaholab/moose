# Friction factor sampling for the PSBT bare-pin square lattice
G_exp0 = -1.6
# Subchannel indices of an interior, an edge, and a corner subchannel
center = 7
edge = 1
corner = 0

!include friction_factor_sampling.i

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 6
    ny = 6
    n_cells = 30
    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = 3
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  friction_closure = 'MATRA'
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
[]
