[presentation]
  style = 'inl'
  [./aux_system]
    type = INLMergeCoverSet
    slide_sets = 'aux_kernels aux_variables'
    title = 'Auxilliary System'
    contents = true
  [../]
  [./aux_kernels]
    type = INLDjangoWikiSet
    wiki = MooseSystems/AuxKernels
    inactive = 'example-10'
  [../]
  [./aux_variables]
    type = INLDjangoWikiSet
    wiki = MooseSystems/AuxVariables
  [../]
[]
