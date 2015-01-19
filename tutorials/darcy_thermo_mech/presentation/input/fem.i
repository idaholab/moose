[presentation]
  style = inl
  title = Finite Elements: The MOOSE Way

  [./fem_cover]
    title = 'Finite Elements: The MOOSE Way'
    type = INLMergeCoverSet
    contents = true
    contents_title = 'FEM Contents'
    slide_sets = 'fem_principles fem_shape fem_numerical'
  [../]

  [./fem_principles]
    type = INLDjangoWikiSet
    wiki = MooseTraining/FEM/Principles

    [./Images]
      [./89]
        width = 420px
      [../]
    [../]
  [../]

  [./fem_shape]
    type = INLDjangoWikiSet
    wiki = MooseTraining/FEM/ShapeFunctions

    [./Images]
      [./93]
        width = 290px
        show_caption = false
      [../]
      [./94]
        width = 290px
        show_caption = false
      [../]
      [./95]
        width = 290px
        show_caption = false
      [../]
      [./96]
        width = 290px
        show_caption = false
      [../]
      [./90]
        width = 235px
        div_center = false
      [../]
      [./91]
        width = 235px
        div_center = false
      [../]
      [./92]
        width = 235px
        div_center = false
      [../]
    [../]
  [../]

  [./fem_numerical]
    type = INLDjangoWikiSet
    wiki = MooseTraining/FEM/NumericalImplementation
  [../]
[]
