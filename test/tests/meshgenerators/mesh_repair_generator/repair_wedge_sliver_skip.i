# A blade PRISM6 wedge standing alone: there is no element across its longest quad side, so there is
# nothing to absorb it into. The repair leaves the (valid, thin) wedge in place and reports it as
# skipped, rather than corrupting the mesh.
[Mesh]
  [wedge]
    type = ElementGenerator
    nodal_positions = '0 0 0  1 0 0  0.5 0.01 0   0 0 1  1 0 1  0.5 0.01 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [repair]
    type = MeshRepairGenerator
    input = wedge
    fix_sliver_elements = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_non_conformality = ERROR
    examine_element_overlap = ERROR
    examine_element_volumes = ERROR
  []
[]
[Problem]
  solve = false
[]
[Executioner]
  type = Steady
[]
