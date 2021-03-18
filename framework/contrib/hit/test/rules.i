[ReplacementRules]

  [./function_dirichlet_to_preset]
    [./Match]
      [./BCs]
        [./<name>]
          type = FunctionDirichletBC
          variable = <var>
          function = <func>
          boundary = <boundary>
        [../]
      [../]
    [../]
    [./Replace]
      [./BCs]
        [./<name>]
          type = FunctionPresetBC
          variable = <var>
          function = <func>
          boundary = <boundary>
        [../]
      [../]
    [../]
  [../]

[]
