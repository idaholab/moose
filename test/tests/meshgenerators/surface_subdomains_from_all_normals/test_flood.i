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
  [assign_subdomains_in_patches]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'sphere_surf_mesh'
    normal_tol = 0.5
    # makes a couple subdomains. Note that we 'painting' over the same
    # elements multiple times because 'flood_elements_once' is not set to true
    max_subdomain_size_centroids = 1.818
  []

  # The ordering of the elements matters for the flooding/painting loops
  allow_renumbering = false
[]
