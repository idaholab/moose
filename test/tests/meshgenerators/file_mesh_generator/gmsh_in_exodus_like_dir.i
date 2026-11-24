[Mesh]
  [file]
    type = FileMeshGenerator
    # The ".e" in the directory name should not cause this Gmsh
    # file to be read with the Exodus reader
    file = gmsh_in.e_dir/sample.msh
  []
[]
