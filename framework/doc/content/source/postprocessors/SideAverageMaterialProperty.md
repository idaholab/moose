# SideAverageMaterialProperty

This post-processor computes the average of a material property over a side set.
The supplied property can be of type `Real`, `std::vector<Real>`,
`RealVectorValue`, `RankTwoTensor`, `RankThreeTensor`, or `RankFourTensor`.
Depending on the property type an index with the correct dimension must be
supplied to select a scalar component from the supplied material property (e.g.
`component = '0 1'` for a property of type `RankTwoTensor`).

!syntax parameters /Postprocessors/SideAverageMaterialProperty

!syntax inputs /Postprocessors/SideAverageMaterialProperty

!syntax children /Postprocessors/SideAverageMaterialProperty
