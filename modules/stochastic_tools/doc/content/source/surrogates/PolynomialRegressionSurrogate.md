# PolynomialRegressionSurrogate

!syntax description /Surrogates/PolynomialRegressionSurrogate

## Overview

This surrogate model takes the vector containing model coefficients ($\textbf{c}$) from [PolynomialRegressionTrainer.md]
and computes the value of the QoI at a new parameter sample by simply evaluating

!equation id=poly_exp
\hat{\textbf{y}}(\textbf{x}') = \sum \limits_{k=1}^{N_p}P(\textbf{x}', \textbf{i}_{k})c_k,

where $\textbf{x}'$ denotes the coordinates of the new sample in the parameter space.
It is important to mention that unlike [NearestPointSurrogate.md],
this surrogate model does not require the evaluation of a function (e.g. distance) for
all training points to determine the new value at $\textbf{x}'$. Thus, for large
training data bases, using a `PolynomialRegressionSurrogate` for repeated
runs is faster.

## Example Input File Syntax

To create a surrogate model which uses polynomial regression, one can use the following syntax:

!listing polynomial_regression/evaluate.i block=Surrogates

It is visible that the data from [PolynomialRegressionTrainer.md] has been saved to `train_out_train.rd`
and the surrogate model is constructed by loading the necessary information from it.
If one wants to do the training and evaluation in the same input, the following syntax can be used:

!listing polynomial_regression/train_and_evaluate.i block=Surrogates

where `train` is the ID of the [PolynomialRegressionTrainer.md] object. For the sampling
of the surrogate model, the same objects can be used in the `Samplers` block:

!listing polynomial_regression/evaluate.i block=Samplers

Finally, a reporter of type [EvaluateSurrogate.md] is created to extract the approximate value of the
QoI(s):

!listing polynomial_regression/evaluate.i block=Reporters

!syntax parameters /Surrogates/NearestPointSurrogate

!syntax inputs /Surrogates/NearestPointSurrogate

!syntax children /Surrogates/NearestPointSurrogate
