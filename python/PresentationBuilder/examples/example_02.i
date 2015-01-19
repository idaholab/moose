[presentation]
  style = 'inl'
  code = 'dark'
  [./cover]
    type = CoverSet
    title = '**PresentationBuilder**<br><br> A tool for generating slides from MOOSE wiki content'
    background-image = 'inl_white_slide.png'
    contents_title = 'Contents'
    contents = true

    [./Slides]
      [./cover-title]
         background-image = 'inl_dark_title.png'
         class = 'middle, cover'
      [../]
    [../]
  [../]

  [./demo]
    type = DjangoWikiSet
    wiki = 'PresentationBuilder'
    background-image = 'inl_white_slide.png'
    contents = true
    contents_level = 2
    title = 'Slides extracted from the mooseframework.org wiki'

    [./Slides]
      [./demo-title]
        class = 'middle, cover'
        background-image = 'inl_dark_title.png'
      [../]
    [../]

    [./Images]
      [./64]
        align = 'center'
        width = '500px'
        download = False
      [../]
    [../]
  [../]

  [./CSS]
    [./remark-code]
      font-size = '14px'
    [../]
    [./cover]
      color = '#ffffff'
      padding-left = '250px'
    [../]
    [./right-column]
      width = '30%'
      float = 'right'
    [../]
    [./left-column]
      width = '65%'
      float = 'left'
    [../]
  [../]
[]
