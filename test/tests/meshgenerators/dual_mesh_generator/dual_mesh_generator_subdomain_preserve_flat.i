[Mesh]
  # The preserved subdomain spans two cells along the interface, so the interface runs straight
  # through the primal node between them. The preserved elements keep that node, so the dual cell
  # beside it has to stop there rather than run from one side midpoint to the next.
  [mycart]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1 1'
    subdomain_id = '3 1
                    3 1'
  []

  [myDualGen]
    type = DualMeshGenerator
    input = mycart
    dual_mesh_type = barycentric
    preserve_primal_subdomains = '3'
  []

  [myDiagnosticGenerator]
    type = MeshDiagnosticsGenerator
    input = myDualGen
    examine_element_overlap = ERROR
    examine_element_volumes = ERROR
    minimum_element_volumes = 0
    examine_non_conformality = ERROR
  []
[]
