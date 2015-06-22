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
    use_wiki_title = false
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
