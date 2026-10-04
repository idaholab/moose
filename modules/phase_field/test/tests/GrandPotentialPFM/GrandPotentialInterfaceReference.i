[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Materials]
  [iface]
    # interfacial free energies and width of cases 1 and 3 of Table 1 in N. Moelans,
    # Mater. Des. 217, 110592 (2022), which with the largest interfacial free energy as
    # reference give kappa = 0.3 and mu = 0.9375
    type = GrandPotentialInterface
    gamma_names = 'g15 g20 g25'
    sigma = '0.15 0.2 0.25'
    width = 1.6
  []
[]

[VectorPostprocessors]
  [mat]
    type = ElementMaterialSampler
    material = iface
    elem_ids = 0
  []
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
