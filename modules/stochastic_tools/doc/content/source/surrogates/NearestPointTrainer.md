# NearestPointTrainer

!syntax description /Trainers/NearestPointTrainer

## Overview

This trainer is meant to produce training data to be used with [NearestPointSurrogate.md]. The data saved (`_sample_points`) is an array of the sample points and the results from the full model:

!equation
\texttt{\_sample\_points} =
\begin{bmatrix}
x_{1,1} & x_{1,2} & \dots  & x_{1,N} \\
x_{2,1} & x_{2,2} & \dots  & x_{2,N} \\
\vdots  & \vdots  & \ddots & \vdots  \\
x_{D,1} & x_{D,2} & \dots  & x_{D,N} \\
y_1     & y_2     & \dots  & y_N
\end{bmatrix}

where $x_{i,j}$ is the point from the sampler at column $i$ and sample $j$, $y_j$ is the result of the full model at sample $j$, $N$ is the number of training samples, and $D$ is the number of sampler columns.  [NearestPointSurrogate.md] then uses this array to find the closest point to the given input point and outputs the full model result.

## Example Input File Syntax

A sampler is created to produce points for the training data:

!listing nearest_point/train.i block=Samplers

The example full model is the `GFunction` vector postprocessor, see [SobolStatistics.md] for more details. This vector postprocessor takes in the sampler and evaluates the function at the training points.

!listing nearest_point/train.i block=VectorPostprocessors

The trainer then takes in the data from the sampler and the results from the vector postprocessor to create the `_sample_points` array:

!listing nearest_point/train.i block=Trainers

We then output the training data to file:

!listing nearest_point/train.i block=Outputs

!syntax parameters /Trainers/NearestPointTrainer

!syntax inputs /Trainers/NearestPointTrainer

!syntax children /Trainers/NearestPointTrainer
