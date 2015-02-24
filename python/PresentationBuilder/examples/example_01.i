[presentation]
  style = inl
  [./basic]
    type = FileSet
    title = '**FileSet:** Build and modify a slide set directly from a markdown file'
    file = example_01.md
    contents = true
    contents_title = 'Contents'

    [./Slides]
      [./test]
        class = 'right, top'
        comments = 'Add comments via the input, rather than in the markdown itself'
      [../]
    [../]

    [./Images]
      [./example_01]
        width = '500px'
        align = 'right'
      [../]
    [../]
  [../]

  [./CSS]
    [./right]
      width = '75%'
      float = 'right'
    [../]
    [./left]
      width = '20%'
      float = 'left'
    [../]
  [../]
[]
