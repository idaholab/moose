[Mesh]
  [mycart]
    type = CartesianMeshGenerator
    dim = 3
    dx = '3 3'
    dy = '4 4'
    dz = '5 5'
    subdomain_id = '1 2
                    3 4

                    5 6
                    7 8'
  []

  [myDualGen]
    type = DualMeshGenerator
    input = mycart
    dual_mesh_type = barycentric
    concave_treatment = 'split polycut netgen'
    preserve_subdomain_interfaces = true
  []

  [convert]
    type = ElementsToSimplicesConverter
    input = myDualGen
  []
[]
