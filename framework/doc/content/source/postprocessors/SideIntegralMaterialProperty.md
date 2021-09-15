# SideIntegralMaterialProperty

This post-processor computes the integral of a material property over a side set.
The supplied property can be of type `Real`, `std::vector<Real>`,
`RealVectorValue`, `RankTwoTensor`, `RankThreeTensor`, or `RankFourTensor`.
Depending on the property type an index with the correct dimension must be
supplied to select a scalar component from the supplied material property (e.g.
`component = '0 1'` for a property of type `RankTwoTensor`).

!syntax parameters /Postprocessors/SideIntegralMaterialProperty

!syntax inputs /Postprocessors/SideIntegralMaterialProperty

!syntax children /Postprocessors/SideIntegralMaterialProperty
