# Advanced Burner Test Reactor - Conversion of Heterogeneous 3D Core to Homogeneous 3D Core using Griffin
# NOTE: This file requires Griffin executable access to run

!include rgmb_abtr_het_mesh.i               # Import mesh input file defined in rgmb_abtr_het_mesh.i

[Mesh]
  [hom_core]
    type = EquivalentCoreMeshGenerator
    input = het_core                   # Name of input heterogeneous mesh
    target_geometry = full_hom         # Target geometry type for
                                       # equivalent core mesh generation
                                       # (options are full_hom, duct_het, and ring_het)
    quad_center_elements = true        # Whether output homogeneous should
                                       # be defined using quad elements (true)
                                       # or tri elements (false)
  []
  data_driven_generator = hom_core     # Optimization needed to bypass explicit heterogeneous mesh generation
  final_generator := hom_core          # Set this line to override the value of final_generator set in rgmb_abtr_het_mesh.i
[]
