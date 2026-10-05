# StatefulMaterialPropertyImporter depends on EXEC_INITIAL ordering. User overrides
# should be rejected before the importer can silently stage data too late.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[UserObjects]
  [importer]
    type = StatefulMaterialPropertyImporter
    file_base = 'unused'
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Steady
[]
