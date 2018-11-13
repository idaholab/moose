[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Materials]
  [./iface]
    # reproduce the parameters from GrandPotentialMultiphase.i
    type = GrandPotentialInterface
    gamma_names = 'gbb gab'
    sigma       = '0.4714  0.6161' # Ratio of 1:1.307 to obtain dihedral angle of 135deg
    width       = 2.8284
  [../]
[]

[VectorPostprocessors]
  [./mat]
    type = MaterialVectorPostprocessor
    material = iface
    elem_ids = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
  execute_on = TIMESTEP_END
[]
