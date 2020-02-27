# CartesianProductSampler

!syntax description /Samplers/CartesianProductSampler

## Overview

The Cartesian product creates a sample for all possible combinations for a set of values,
$Z = A \times B \times \cdots \times L \times \ M$. For example, given the vectors
$\vec{x_0} = (1,2)$, $\vec{x_1} = (11,12,13)$, and $\vec{x_2} = (21,22)$. The resulting Cartesian
product (Z) is:

!equation
Z = \begin{bmatrix}
    1, 11, 21 \\
    1, 11, 22 \\
    1, 12, 21 \\
    1, 12, 22 \\
    1, 13, 21 \\
    1, 13, 22 \\
    2, 11, 21 \\
    2, 11, 22 \\
    2, 12, 21 \\
    2, 12, 22 \\
    2, 13, 21 \\
    2, 13, 22 \\
    \end{bmatrix}


## Implementation

The CartesianProductSampler utilizes what is referred to as a "lazy" scheme for the calculation of
the $Z$ matrix, using the algorithm below. This simply means that any given row ($n$) in the matrix
can be computed directly without the need for computing the values prior to the entry. As such the
sampling works efficiently in parallel to create distributed sample data.

!equation
Z(n) = \begin{bmatrix}
       A [ [\frac{n}{|B||C|\cdots|L||M|}] \textrm{mod} |A|, \\
       B [ [\frac{n}{|C|\cdots|L||M|}] \textrm{mod} |B|, \\
       \vdots \\
       L [ [\frac{n}{|M|}] \textrm{mod} |L|, \\
       M [ [\frac{n}{1}] \textrm{mod} |M| \\
       \end{bmatrix}

## Example Input File Syntax

The following input file snippet demonstrates the creation of a CartesianProductSampler object
with three variables. The variables are provided using triplets that provide the starting point,
the stepsize, and the number of steps. For example, the triplet of `10 1.5 3` result in
$(10, 11.5, 13)$.

!listing cartesian_product/grid.i block=Samplers/sample

The resulting Cartesian product is provided in the output file, as shown below.

!listing cartesian_product/gold/grid_out_data_0000.csv

!syntax parameters /Samplers/CartesianProductSampler

!syntax inputs /Samplers/CartesianProductSampler

!syntax children /Samplers/CartesianProductSampler
