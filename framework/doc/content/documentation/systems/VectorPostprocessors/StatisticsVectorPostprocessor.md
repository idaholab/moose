# StatisticsVectorPostprocessor

## Short Description

!syntax description /VectorPostprocessors/StatisticsVectorPostprocessor

## Description

The `StatisticsVectorPostprocessor` computes statistical information for each column of another `VectorPostprocessor` (VPP).  The results are output in columns corresponding to the column-names of the original VPP.  The rows of each column are the statistical measures the `StatisticsVectorPostprocessor` was asked to compute.  In addition, the first column is named `stat_type` and contains the unique identifier for the type of statistical measure computed in that row.

The statistical measures are chosen using the `stats` input parameter.  Note that multiple statistical measures can be computed simultaneously by passing in more than one to the `stats` input parameter.  The current statistical measures (and their `stat_type` identifier) the `StatisticsPostprocessor` can compute are:

### Min: 0

`stats = min`

The minimum value within the column.

### Max: 1

`stats = max`

The maximum value within the column

### Sum: 2

`stats = sum`

The sum of the column

\begin{equation}
\Sigma = \sum_{i=1}^N{Vi}
\end{equation}

### Average: 3

`stats = average`

The average (mean) of the column

\begin{equation}
\bar{V} = \frac{\sum_{i=1}^{N}{V_i}}{N}
\end{equation}

### Standard Deviation: 4

`stats = stddev`

The standard deviation of the values

\begin{equation}
\sigma = \sqrt{\frac{\sum_{i=1}^{N}{(V_i - \bar{V})^2}}{N-1}}
\end{equation}

### The 2-Norm (Eucliden Norm): 5

`stats = norm2`

The 2-norm (also known as the Euclidean Norm or the "distance")

\begin{equation}
|V|_2 = \sqrt{\sum_{i=1}^{N}{{V_i}^2}}
\end{equation}


## Important Notes

Note that this VPP only computes on processor 0.  This is because that's all that is necessary for output - and the VPP it uses for its values may be doing the same.

!syntax parameters /VectorPostprocessors/StatisticsVectorPostprocessor

!syntax inputs /VectorPostprocessors/StatisticsVectorPostprocessor

!syntax children /VectorPostprocessors/StatisticsVectorPostprocessor

!bibtex bibliography
