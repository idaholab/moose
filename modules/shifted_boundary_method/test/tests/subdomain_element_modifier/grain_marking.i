# Marks a background mesh with grain (subdomain) IDs using SubdomainElementModifier.
#
# The boundary mesh 'grain_5_boundary.msh' holds five grain-boundary loops
# (subdomains 1-5) that tile the domain [-0.6, 0.6] x [-0.4, 0.4] and meet at
# interior corners. SubdomainElementModifier assigns each background element to
# the grain that contains it; elements straddling a grain corner are assigned to
# the grain occupying the largest active-area fraction (ties broken by lowest
# subdomain ID). The resulting per-subdomain (element block) assignment is written
# to Exodus for comparison.

[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = 'grain_5_boundary.msh'
    save_with_name = 'boundary_mesh'
  []
  [background_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.6
    xmax = 0.6
    ymin = -0.4
    ymax = 0.4
    nx = 9
    ny = 6
    subdomain_ids = 0
  []
  final_generator = background_mesh
  add_subdomain_ids = '1 2 3 4 5'
[]

# Unused variable so the Exodus output has a field to write; the test compares the
# per-element subdomain (block) assignment produced by the mesh modifier.
[Variables]
  [u]
  []
[]

[UserObjects]
  [surface_builder]
    type = SurfaceMeshBySubdomainBuilder
    surface_mesh = boundary_mesh
  []
  [subdomain_tester]
    type = PointInSubdomainCheckUO
    builder = surface_builder
  []
[]

[MeshModifiers]
  [assign_subdomains]
    type = SubdomainElementModifier
    subdomain_id_tester = subdomain_tester
    lambda = 0.5
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
