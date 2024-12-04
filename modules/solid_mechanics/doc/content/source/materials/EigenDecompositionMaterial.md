# EigenDecompositionMaterial

!syntax description /Materials/EigenDecompositionMaterial

## Description

This class reads in a symmetric `RankTwoTensor` material given by [!param](/Materials/EigenDecompositionMaterial/rank_two_tensor) and performs an eigendecomposition on it.  The results of the decomposition are scalar materials named `max_eigen_value`, `mid_eigen_value`, and `min_eigen_value` and the corresponding vector material properties named `max_eigen_vector`, `mid_eigen_vector`, and `min_eigen_vector`. These names can be preceeded by the [!param](/Materials/EigenDecompositionMaterial/base_name) to allow for more than one `EigenDecompositionMaterial` per block.  These material properties can be output using the Material Outputs system with the [!param](/Materials/EigenDecompositionMaterial/output_properties) as shown in the below example.  This material is useful for visualizing the elemental maximum principal stress and direction as a vector.  An error will be produced if an unsymmetric `RankTwoTensor` is decomposed which can happen for the deformation gradient.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/eigen_decomp_material/prescribed_strain_3D.i block=Materials/nonADeig_decomp

!syntax parameters /Materials/EigenDecompositionMaterial

!syntax inputs /Materials/EigenDecompositionMaterial

!syntax children /Materials/EigenDecompositionMaterial
