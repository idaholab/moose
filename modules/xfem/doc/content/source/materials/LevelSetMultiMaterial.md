# LevelSetMultiMaterial

The levelset multi material system switches material properties based on a set of levelset functions.
Each levelset function $\phi$ is expected to define a material interface at $\phi^{-1}(0)$. The user should
provide a dictionary (explained below) of mapped material properties based on combinations of the signs of the levelset functions.

The types of material properties on the two sides of a material interface are assumed to be the same. Supported types and their corresponding
MooseObjects are

| Data type      | Material                              | AD Material                             |
| -------------- | ------------------------------------- | --------------------------------------- |
| Real           | `LevelSetMultiRealMaterial`           | `ADLevelSetMultiRealMaterial`           |
| RankTwoTensor  | `LevelSetMultiRankTwoTensorMaterial`  | `ADLevelSetMultiRankTwoTensorMaterial`  |
| RankFourTensor | `LevelSetMultiRankFourTensorMaterial` | `ADLevelSetMultiRankFourTensorMaterial` |

## Example Input File Syntax

Consider a simple diffusion problem on a square domain consisting of two non-intersecting material interfaces defined by two levelset functions. The domain is divided into three subdomains by the two interfaces. Each subdomain has a different diffusion coefficient.

!media xfem/levelsets.png
       style=width:60%;float:center;padding-top:2.7%;
       caption=Two levelset functions defining two material interfaces

The three diffusion coefficients share the same material property name "diffusion_coefficient" but have different base names "A", "B" and "C", for example:

!listing xfem/test/tests/multimaterials/glued_multimaterials_2d.i start=[diffusivity_A] end=[diff_combined]

Based on the signs of the levelset functions, the three diffusion coefficients can be combined into one using the following dictionary:

!media xfem/dictionary.png
       style=width:60%;float:center;padding-top:2.7%;
       caption=A map from combinations of the levelset signs to base names

The corresponding syntax in the input file would look like:

!listing xfem/test/tests/multimaterials/glued_multimaterials_2d.i block=Materials/diff_combined
