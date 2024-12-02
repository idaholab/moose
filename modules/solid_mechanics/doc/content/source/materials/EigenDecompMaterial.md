# EigenDecompMaterial

!syntax description /Materials/EigenDecompMaterial

## Description

This class reads in a Rank two tensor material given by [!param](/Materials/EigenDecompMaterial/rank_two_tensor) and performs an eigendecomposition on it.  The results of the decomposition are scalar materials named `max_eigen_value`, `mid_eigen_value`, and `min_eigen_value` and the corresponding vector material properties named `max_eigen_vector`, `mid_eigen_vector`, and `min_eigen_vector`. These names can be preceeded by the [!param](/Materials/EigenDecompMaterial/base_name) to allow for more than one `EigenDecompMaterial` per block.  These material properties can be output using the Material Outputs system with the [!param](/Materials/EigenDecompMaterial/output_properties) as shown in the below example.  This material is useful for visualizing the elemental maximum principal stress and direction as a vector.

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/eigen_decomp_material/prescribed_strain_3D.i block=Materials/nonADeig_decomp

!syntax parameters /Materials/EigenDecompMaterial

!syntax inputs /Materials/EigenDecompMaterial

!syntax children /Materials/EigenDecompMaterial
