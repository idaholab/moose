# Generates meshes used by hs_coupler_2d3d.i.
#
# Run as
#   myapp-opt -i mesh.i --mesh-only mesh.e
# to build the standard mesh and for the fine mesh change "n_elems_azimuthal"
# to "13" and run as
#   myapp-opt -i mesh.i --mesh-only mesh_fine.e

R_inner = 0.005
R_outer = 0.01
length = 0.5

n_elems_axial = 10
n_elems_radial = 5
n_elems_azimuthal = 10

[Mesh]
  [ring]
    type = AnnularMeshGenerator
    nr = ${n_elems_radial}
    nt = ${n_elems_azimuthal}
    rmin = ${R_inner}
    rmax = ${R_outer}
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = ring
    heights = ${length}
    num_layers = ${n_elems_axial}
    direction = '0 0 1'
  []
[]

[Outputs]
  exodus = true
[]
