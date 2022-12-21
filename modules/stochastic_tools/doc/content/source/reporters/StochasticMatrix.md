# StochasticMatrix

!syntax description /Reporters/StochasticMatrix

## Overview

The primary purpose of the StochasticMatrix object is to output sampler data alongside stochastic simulation results. The object retrieves the sampler specified with [!param](/Reporters/StochasticMatrix/sampler) and outputs a vector for each column of the sampler matrix. The names of these vectors can be specified using [!param](/Reporters/StochasticMatrix/sampler_column_names). Similarly to [StochasticReporter.md], this object can receive stochastic data from objects like [SamplerReporterTransfer.md]. However, the sampler used in these other objects, like the transfer, must utilize the same sampler as the one specified in [!param](/Reporters/StochasticMatrix/sampler).

## Example Syntax

The following is an example of using StochasticMatrix to output sampler data and results from stochastic simulations. Here we create an arbitrary matrix using [InputMatrixSampler.md] which used to run stochastic simulations using [SamplerFullSolveMultiApp.md].

!listing modules/stochastic_tools/test/tests/reporters/stochastic_matrix/stochastic_matrix.i block=Samplers MultiApps

The parameters are transferred via a [SamplerParameterTransfer.md] and the results are tranferred back with a [SamplerReporterTransfer.md]. This transfers the results in to the StochasticMatrix object named `matrix`.

!listing modules/stochastic_tools/test/tests/reporters/stochastic_matrix/stochastic_matrix.i block=Transfers Reporters

The input parameters in the StochasticMatrix block specify the column names, and the resulting CSV file looks like:

!listing modules/stochastic_tools/test/tests/reporters/stochastic_matrix/gold/stochastic_matrix_out_matrix_0001.csv

!syntax parameters /Reporters/StochasticMatrix

!syntax inputs /Reporters/StochasticMatrix

!syntax children /Reporters/StochasticMatrix
