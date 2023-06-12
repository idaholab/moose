# Cartesian1D

!syntax description /Samplers/Cartesian1D

## Overview

Cartesian1D is similar to [CartesianProduct](CartesianProductSampler.md).
But instead of creating a sample for all possible combinations for a set of values,
it samples each set of values one at a time.
For example, given the vectors $\vec{x_0} = (1,2)$, $\vec{x_1} = (11,12,13)$, and $\vec{x_2} = (21,22)$ and nominal values $\bar{x_0} = 1.5$, $\bar{x_1} = 12$, and $\bar{x_2} = 21.5$,
the resulting sampling matrix is:

!equation
Z = \begin{bmatrix}
    1, 12, 21.5 \\
    2, 12, 21.5 \\
    1.5, 11, 21.5 \\
    1.5, 12, 21.5 \\
    1.5, 13, 21.5 \\
    1.5, 12, 21 \\
    1.5, 12, 22 \\
    \end{bmatrix}

## Example Input File Syntax

The following input file snippet demonstrates the creation of a Cartesian1D object
with three variables. The variables are provided using triplets that provide the starting point,
the stepsize, and the number of steps. For example, the triplet of `10 1.5 3` result in
$(10, 11.5, 13)$.

!listing cartesian_1D/grid.i block=Samplers

The resulting Cartesian sampling is provided in the output file, as shown below.

!listing cartesian_1D/gold/grid_out_data_0000.csv

!syntax parameters /Samplers/Cartesian1D

!syntax inputs /Samplers/Cartesian1D

!syntax children /Samplers/Cartesian1D
