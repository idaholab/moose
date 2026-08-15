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

  [convert]
    type = ElementsToSimplicesConverter
    input = myDualGen
  []
[]
