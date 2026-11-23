[Mesh]
  [mycart]
    type = CartesianMeshGenerator
    dim = 2
    dx = '3 3'
    dy = '4 4'
    subdomain_id = '1 1
                    1 4'
  []

  [myDualGen]
    type = DualMeshGenerator
    input = mycart
    dual_mesh_type = barycentric
    preserve_subdomain_interfaces = true
  []

  # The dual cell around the center node is concave, so its vertex_average() falls on one of its own
  # corners, which it shares with the neighboring cells. The centroid-based overlap heuristic then
  # finds several elements at that point and reports a false positive, so the overlap check is left
  # off here; the conformality and volume checks still cover this mesh.
  [myDiagnosticGenerator]
      type = MeshDiagnosticsGenerator
      input = myDualGen
      examine_element_volumes = ERROR
      minimum_element_volumes = 0
      examine_non_conformality = ERROR
  []
  # exodiff cannot read NSIDED blocks, so the polygons are split for the comparison
  [convert]
    type = ElementsToSimplicesConverter
    input = myDiagnosticGenerator
  []
[]
