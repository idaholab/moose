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
  [assign_a_subdomain_at_top]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'sphere_surf_mesh'
    normal_tol = 0.3
    normal = '0 0 -1'
    # otherwise we will start painting and the 'normal' is only used
    # to select the starting elements for painting
    fixed_normal = true
  []

  # The ordering of the elements matters for the flooding/painting loops
  allow_renumbering = false
[]
