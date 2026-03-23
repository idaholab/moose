[Mesh]
  [sphere_vol]
    type = SphereMeshGenerator
    nr = 1
    radius = 2
  []
  [sphere_surf_sub]
    type = LowerDBlockFromSidesetGenerator
    input = 'sphere_vol'
    sidesets = '0'
    new_block_name = 'sphere_surf'
  []
  [sphere_surf_mesh]
    type = BlockToMeshConverterGenerator
    input = 'sphere_surf_sub'
    target_blocks = 'sphere_surf'
  []
  [two_spheres]
    type = CombinerGenerator
    inputs = 'sphere_surf_mesh'
    positions = '0 0 0
                 2 0 0'
  []
  [assign_subdomains]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'two_spheres'
    # loose enough to paint over each coarse sphere
    normal_tol = 0.45
    contiguous_assignments_only = true
    # no need to paint over each sphere 24 times
    flood_elements_once = true
  []

  # The ordering of the elements matters for the flooding/painting loops
  allow_renumbering = false
[]
