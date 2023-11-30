# VectorDotProduct

!syntax description /Reporters/VectorDotProduct

## Overview

`VectorDotProduct` performs a dot product on two reporters, [!param](/Reporters/VectorDotProduct/vector_a) and [!param](/Reporters/VectorDotProduct/vector_b), of `std::vector<Real>` and scales them by [!param](/Reporters/VectorDotProduct/scale).  This reporter was created to compute the objective function from the misfit reporter created by [OptimizationData.md].  The scalar reporter created can then be transferred as the objective value into [GeneralOptimization.md].

!syntax parameters /Reporters/VectorDotProduct

!syntax inputs /Reporters/VectorDotProduct

!syntax children /Reporters/VectorDotProduct
