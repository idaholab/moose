[presentation]
  style = 'inl'
  [./input_file]
    type = INLDjangoWikiSet
    wiki = MooseTraining/InputFile
    contents = false
    title = 'MOOSE Input File'
  [../]
  [./mesh]
    type = INLDjangoWikiSet
    wiki = MooseSystems/Mesh
    contents = false
    title = 'Mesh Block'
  [../]
  [./modifiers]
    type = INLDjangoWikiSet
    wiki = MooseSystems/MeshModifiers
    contents = false
    title = 'Mesh Modifiers'
  [../]
  [./outputs]
    type = INLDjangoWikiSet
    wiki = MooseSystems/Outputs
    contents = false
    title = 'Outputs'
  [../]
  [./anatomy]
    type = INLDjangoWikiSet
    wiki = MooseTraining/MooseObject
    contents = false
    title = 'Anatomy of a MOOSE Object'
  [../]
[]
