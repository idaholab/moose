# Generates a mesh to be used by main.i. Run with --mesh-only:
#
#   thermal_hydraulics-opt -i mesh.i --mesh-only mesh.e

length = 5.0
inner_radius = 0.01
outer_radius = 0.02

n_elems_axial = 10
n_elems_radial = 5
n_elems_azimuthal = 10

[Mesh]
  [annular]
    type = AnnularMeshGenerator
    nr = ${n_elems_radial}
    nt = ${n_elems_azimuthal}
    rmin = ${inner_radius}
    rmax = ${outer_radius}
    growth_r = 1.0
  []
  [extruder]
    type = AdvancedExtruderGenerator
    input = annular
    heights = '${length}'
    num_layers = '${n_elems_axial}'
    direction = '0 0 1'
  []
  [rename_boundary]
    type = RenameBoundaryGenerator
    input = extruder
    old_boundary = '0'
    new_boundary = 'inner'
  []
[]
