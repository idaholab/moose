# InterceptedElementModifier requires either an 'in_out_test' user object or a
# 'signed_dist_function' to classify elements. This input supplies neither, so
# initialSetup() must error out. Used as a RunException coverage test for that
# configuration guard.

[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    subdomain_ids = 1
  []
  add_subdomain_ids = '2'
[]

[MeshModifiers]
  [intercepted]
    type = InterceptedElementModifier
    subdomain_id_inside = 1
    subdomain_id_outside = 2
    is_domain_inside_surface = true
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Steady
[]
