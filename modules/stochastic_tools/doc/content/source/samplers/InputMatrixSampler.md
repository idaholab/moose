# InputMatrix

!syntax description /Samplers/InputMatrix

## Overview

This sampler simply takes a sampling matrix from input using the [!param](/Samplers/InputMatrix/matrix) parameter.

## Example Input Syntax

See the following as an example input that creates a matrix with five rows and six columns:

!listing input_matrix/input_matrix.i block=Samplers

The resulting matrix looks like the following in CSV format:

!listing input_matrix/gold/input_matrix_out_data_0001.csv

!syntax parameters /Samplers/InputMatrix

!syntax inputs /Samplers/InputMatrix

!syntax children /Samplers/InputMatrix
