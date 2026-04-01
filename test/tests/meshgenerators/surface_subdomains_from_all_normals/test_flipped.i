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
    show_info = true
  []
  [flip_these_elements]
    type = OrientSurfaceMeshGenerator
    input = 'assign_a_subdomain_at_top'
    included_subdomains = '2'
    normal_to_align_with = '0 0 1'
  []
  [repaint_over_ignoring_flips]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'flip_these_elements'
    # large enough to paint over the entire sphere, from wherever we start
    normal_tol = 0.45
    # no need to paint elements multiple times
    flood_elements_once = true
    # ignore the orientation flips
    consider_flipped_normals = true
    flipped_normal_tol = 0.45
    # flip the elements too (not required, but can help downstream generators)
    flip_inverted_normals = true
  []

  # The ordering of the elements matters for the flooding/painting loops
  allow_renumbering = false
[]
