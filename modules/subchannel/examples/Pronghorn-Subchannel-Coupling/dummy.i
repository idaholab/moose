[Mesh]
    [fmesh]
      type = FileMeshGenerator
      file = 'seven_wrapper_interwrapper_in.e'
    []

    [delete]
      type = BlockDeletionGenerator
      block = 'porous_flow interwrapper'
      input = fmesh
    []
  []

  [Problem]
    solve = false
  []

  [AuxVariables]
    [T_wrapper]
      type = MooseVariableFVReal
    []
  []

  [Executioner]
    type = Transient
  []

