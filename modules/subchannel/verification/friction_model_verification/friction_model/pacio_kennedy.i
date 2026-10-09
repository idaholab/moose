# 127-pin MYRRHA bundle of Kennedy et al. (2015), Table 6 of Pacio et al. (2022). The default closures are PCTD friction and mixing;
# for UCTD friction and Chen-Todreas (1986) mixing, override
# SCMClosures/Chen/friction_model=Upgraded SCMClosures/Chen_Todreas/mixing_model=1986
# SCMClosures/Chen_Todreas/CT=0.0 on the command line.

# Inlet mass flux from 10^G_exp0 = 40 kg/m^2-s, a bulk Reynolds number of about 466, up to 10^(G_exp0 +
# 0.1 n_steps) = 3981 kg/m^2-s, a bulk Reynolds number of about 46340
G_exp0 = 1.6
n_steps = 20
n_rings = 7
pin_diameter = 0.00655 # m
# The wire diameter of Table 6 of Pacio et al. (2022), 1.80 mm, exceeds the pin-to-pin gap,
# P - D = 1.79 mm, by 0.01 mm, which the mesh generator rejects, so it is reduced to the gap
wire_diameter = 0.00179 # m
pitch = 0.00834 # m
wire_lead = 0.265 # m
# Flat-to-flat distance from Eq. (A.6) of Pacio et al. (2022), FF = n P sqrt(3) + 2 W - D, with
# n = n_rings - 1 and the edge pitch W = 8.377 mm
flat_to_flat = 0.096876 # m
# Subchannel indices of an interior, an edge, and a corner subchannel. The interior subchannels
# come first, 6 n^2 of them, followed by the edge and corner subchannels of the outermost ring
center = 0
edge = 216
corner = 217
# Number of interior, edge, and corner subchannels: 6 n^2, 6 n, and 6
n_center = 216
n_edge = 36
n_corner = 6

!include pacio_paper.i
!include pacio_kennedy_mdot.i

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    nrings = ${n_rings}
    n_cells = 30
    flat_to_flat = ${flat_to_flat}
    heated_length = 3
    pin_diameter = ${pin_diameter}
    pitch = ${pitch}
    dwire = ${wire_diameter}
    hwire = ${wire_lead}
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]
