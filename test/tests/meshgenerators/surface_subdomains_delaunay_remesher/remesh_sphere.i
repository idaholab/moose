[Mesh]
  # Create a base sphere surface mesh
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

  # Divide it up in patches that can be flattened
  [create_patches]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'sphere_surf_mesh'
    normal_tol = 0.3
    # so we limit the size of the patches to a certain angle range
    fixed_normal = true
    # no need to paint the sphere multiple times
    flood_elements_once = true
  []

  # Remesh the patches using triangle elements
  [remesh]
    type = SurfaceSubdomainsDelaunayRemesher
    input = 'create_patches'
    # keep the subdomains in this test
    avoid_merging_subdomains = true

    verbose = true
  []

  # Diagnostics
  [diag]
    type = MeshDiagnosticsGenerator
    input = remesh
    examine_element_overlap = WARNING
    examine_element_types = WARNING
    examine_element_volumes = WARNING
    examine_non_conformality = WARNING
    examine_nonplanar_sides = INFO
    examine_sidesets_orientation = WARNING
    search_for_adaptivity_nonconformality = WARNING
    check_local_jacobian = WARNING
  []

  # The ordering of the elements matters for the flooding/painting loops
  allow_renumbering = false
[]
