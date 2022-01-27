# NestedMonteCarlo

!syntax description /Samplers/NestedMonteCarlo

## Overview

This sampler is meant to emulate a nested loop of Monte Carlo samples. Columns of the sampling matrix are only sampled with a frequency based on which loop they are a part of. Columns associated with the inner most loop are sampled at each row and columns associated with outer loops are sampled once its inner loop is complete. The number of rows in each loop is defined by the [!param](/Samplers/NestedMonteCarlo/num_rows) parameter, where the first entry is the outer most loop and the last entry is the inner loop. Each of the entries in this parameter must be associated with a set of distributions specified with [!param](/Samplers/NestedMonteCarlo/distributions) which define the sampler columns.

## Example Input Syntax

The following example defines a Monte Carlo sampler with three nested loops. The outer loop has 10 samples, the middle has 5, and the inner has 2. The columns are defined by 5 distributions, the first three are associated with the outer loop, the second 2 with the middle, and the last one with the inner.

!listing modules/stochastic_tools/test/tests/samplers/nested_monte_carlo/nested_monte_carlo.i block=Samplers

The resulting matrix is 5 columns and 100 rows ($10\times 5 \times 2$), which is shown below. As a result, the fifth column changes every row, the third and fourth column changes every 2 rows, and the firth 3 columns change every 10 rows.

!listing modules/stochastic_tools/test/tests/samplers/nested_monte_carlo/gold/nested_monte_carlo_out_data_0001.csv

!syntax parameters /Samplers/NestedMonteCarlo

!syntax inputs /Samplers/NestedMonteCarlo

!syntax children /Samplers/NestedMonteCarlo
