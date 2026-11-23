[Mesh]
  [mycart]
    type = CartesianMeshGenerator
    dim = 2
    dx = '3 3'
    dy = '4 4'
    subdomain_id = '1 1
                    1 4'
  []

  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = mycart
    primary_block = 1
    paired_block = 4
    new_boundary = 'interface'
  []

  [myDualGen]
    type = DualMeshGenerator
    input = interface
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

  # exodiff cannot read NSIDED blocks, so the polygons are split for the comparison
  [convert]
    type = ElementsToSimplicesConverter
    input = myDiagnosticGenerator
  []

  # The sideset orientation check does not support polygons, so it runs on the split mesh
  [checkSidesets]
    type = MeshDiagnosticsGenerator
    input = convert
    examine_sidesets_orientation = ERROR
  []
[]
