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

  [myDiagnosticGenerator]
      type = MeshDiagnosticsGenerator
      input = myDualGen
      examine_element_overlap = ERROR
      examine_element_volumes = ERROR
      minimum_element_volumes = 0
      examine_non_conformality = ERROR
  []
[]
