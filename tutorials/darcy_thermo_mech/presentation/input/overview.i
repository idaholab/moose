[presentation]
  style = 'inl'
  [./overview_cover]
    title = 'MOOSE Overview'
    type = INLMergeCoverSet
    contents = true
    slide_sets = 'overview moose-papers'
  [../]

  [./overview]
    type = INLDjangoWikiSet
    wiki = MooseTraining/Overview
    non_ascii_warn = false
    [./Slides]
      [./moose]
        auto_title = False
        class = bottom
      [../]

      [./application-arch]
        auto_title = False
        class = bottom
      [../]
    [../]

    [./Images]
      align = 'center'
      [./73]
        width = '700px'
      [../]
      [./74]
        width = '675px'
      [../]
      [./75]
        width = '425px'
      [../]
      [./76]
        width = '250px'
      [../]
      [./77]
        width = '250px'
      [../]
      [./78]
        width = '250px'
      [../]
      [./79]
        width = '250px'
      [../]
    [../]
  [../]

  [./moose-papers]
    type = INLDjangoWikiSet
    wiki = MoosePublications
    non_ascii_warn = false
    [./Slides]
      [./publications]
        prefix = '# MOOSE Results'
      [../]
    [../]
  [../]
[]
