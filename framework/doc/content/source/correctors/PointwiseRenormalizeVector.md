# PointwiseRenormalizeVector

!syntax description /Correctors/PointwiseRenormalizeVector

This `Corrector` can pointwise renormalize the solution for a set of variables, taking each variable as the component of a vector and scaling the variables to obtain the user specified L2-norm.

## Applications

For example in a micromagnetics simulation the magnetization director field is a vector field that should stay normalized, however the evolution equations might not be strictly norm conserving, requiring a renormalization at the end of each time step to avoid drift on the norm.

## Design

The PointwiseRenormalizeVector corrector is derived from `GeneralUserObject` and iterates over all active local elements. On each element the DOF indices for all coupled variables are obtained. Starting with the first index for each variable we check of the DOF is local to the current processor and assemble the corresponding value from each variable into a vector. The L2-norm is calculated and the vector renormalized for the norm to match [!param](/Correctors/PointwiseRenormalizeVector/norm), unless all solution values are zero. This is repeated for all remaining DOF indices and for the old and older solution states.

!syntax parameters /Correctors/PointwiseRenormalizeVector

!syntax inputs /Correctors/PointwiseRenormalizeVector

!syntax children /Correctors/PointwiseRenormalizeVector

!bibtex bibliography
