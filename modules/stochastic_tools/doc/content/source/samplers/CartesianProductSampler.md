# CartesianProductSampler

!syntax description /Samplers/CartesianProductSampler

## Overview

The Cartesian product creates a sample for all possible combinations for a set of values,
$Z = A \times B \times \cdots \times L \times \ M$. The calculation is being performed in parallel
using a "lazy" scheme to all for the $n_th$ value to be computed exactly as follows

!equation
Z(n) = \begin{bmatrix}
       A [ [\frac{n}{|B||C|\cdots|L||M|}] \textrm{mod} |A|, \\
       B [ [\frac{n}{|C|\cdots|L||M|}] \textrm{mod} |B|, \\
       \vdots \\
       L [ [\frac{n}{|M|}] \textrm{mod} |L|, \\
       M [ [\frac{n}{1}] \textrm{mod} |M| \\
       \end{bmatrix}


## Example Input File Syntax

!! Describe and include an example of how to use the CartesianProductSampler object.

!syntax parameters /Samplers/CartesianProductSampler

!syntax inputs /Samplers/CartesianProductSampler

!syntax children /Samplers/CartesianProductSampler
