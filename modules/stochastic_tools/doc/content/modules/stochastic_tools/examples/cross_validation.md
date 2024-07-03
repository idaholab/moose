# K-Fold Cross Validation

K-fold cross validation options for `SurrogateTrainer` are in development. In k-fold cross validation, the sampler data is divided into $k$ non-overlapping folds. The model is retrained $k$ times, in each instance with a different fold held back for testing. In this way, each predictor/response pair is used in training $k-1$ models, and used for evaluation once. The performance of the model in predicting responses to the held back values is reported as the root-mean-square error (RMSE).

!equation
\text{RMSE} = \sqrt{\frac{\sum_{i=1}^{n}(\hat{y}_i - y_i)^2}{n}},

Currently, this tool has only been demonstrated for [PolynomialRegressionTrainer.md], but demonstrations for other trainers (such as [PolynomialChaosTrainer.md]) will be made available in the near future. This example is meant to demonstrate how the cross validation capabilities in the `SurrogateTrainer` class are used. This example builds on [modules/stochastic_tools/examples/poly_regression_surrogate.md], using the same physical problem and uncertain parameters. See that example for further information on setting up the training and model evaluation for this problem.

!include surrogate_training.md start=heat_conduction_model_begin end=heat_conduction_model_finish

## Cross Validation

To perform cross validation, a [SurrogateModel](Surrogates/index.md) must be included in the input file along with the trainer. This is used to calculate predictions for the holdout set.  The following options can be used to control the cross validation routine:

- [!param](/Trainers/PolynomialRegressionTrainer/cv_n_trials): The number of repeated cross validation trials to perform.  This option can be used to better estimate the performance of the model
- [!param](/Trainers/PolynomialRegressionTrainer/cv_splits): The number of splits (k) to use in cross validation.
- [!param](/Trainers/PolynomialRegressionTrainer/cv_surrogate): The `SurrogateModel` object to use for evaluating error compared to the test data for each split.
- [!param](/Trainers/PolynomialRegressionTrainer/cv_type): The type of cross-validation to perform. Currently, the only options are `none` or `k_fold`.

The following input file snippet provides an example of performing repeated 5-fold cross validation for 100 trials using a [PolynomialRegressionTrainer.md] and [PolynomialRegressionSurrogate.md], for the example one-dimensional heat conduction model used in [/examples/surrogate_training.md]. Please refer to the documentation for this model type for details on other options. It is also important to note that the values in `GlobalParams` could have been set in `Trainers/pr_max` instead.

!listing stochastic_tools/examples/surrogates/cross_validation/all_trainers_uniform_cv.i block=GlobalParams Distributions Samplers Trainers Surrogates remove=Trainers/pc_max Trainers/np_max Trainers/gp_max Trainers/ann_max Surrogates/pc_surr Surrogates/np_surr Surrogates/gp_surr Surrogates/ann_surr replace=['num_rows = 1000','num_rows = 6560']

## Results and Analysis

In this section, cross validation results for uniform and normal parameter distributions are provided. Here, we've only trained models for $T_{max}$ for simplicity. A short analysis of the results is provided
as well to showcase potential issues the user might encounter when using polynomial regression.

For reference, results for $T_{max}$ from [modules/stochastic_tools/examples/poly_regression_surrogate.md] are summarized in [!ref](ref_results).

!table id=ref_results caption=The reference results for the mean and standard deviation of the maximum temperature.
| Moment | Uniform | Normal |
| :- | - | - |
| $\mu_{T_{max}}$ | 301.3219 | 301.2547 |
| $\sigma_{T_{max}}$ | 5.9585 | 10.0011 |

### Uniform parameter distributions

First, we examine results from cross validation of a third degree polynomial regression for uniformly distributed parameters. For comparison, 2-fold, 5-fold, and 10-fold cross validation was used. Because $k$-fold CV does not test every possible splitting of the training data, the resulting RMSE can vary significantly depending on the splits used. To better estimate the model's performance, repeated cross validation can be performed. For each $k$, cross validation was repeated $n$=1e5 times to obtain a more representative RMSE across the set of trials - for this example, the mean and standard deviation were calculated. The results of these trials are summarized in [!ref](stats_uniform). For this learning problem, the cross validation results seem to support the use of a third degree polynomial regression - in all cases, the mean RMSE was less than 0.1% of the mean $T_{max}$

!table id=stats_uniform caption=Mean and standard deviation of RMSE scores obtained for 1e5 repeated cross validation trials, for uniform parameter distributions.
| Moment           | 2-fold    | 5-fold      | 10-fold   |
| :-               | -         | -           | -         |
| $\mu_{RMSE}$     | 0.2496    | 0.2484      | 0.2483    |
| $\sigma_{RMSE}$  | 0.0017    | 6.5e-4      | 4.1e-4    |

Distributions of the RMSE obtained from repeated cross validation are shown in figure 1. For larger $k$, the size of each fold decreases and the model has access to more of the training data. Because of this, we expect less variance in the resulting RMSE scores for greater $k$. This is reflected in the plot. For 2-fold cross validation, the distribution is wide, indicating that any given trial of 2-fold CV may provide a poor measure of model performance compared to 5- or 10-fold CV. As a trade off, increasing $k$ increases training expense - more models must be trained on larger subsets. These factors should be kept in mind when cross validating a surrogate model.

!media stochastic_tools/surrogates/cross_validation/cv_uniform_1dheat.svg id=cv_uniform style=width:70% caption=Distribution of RMSE reported from 1e5 repetitions of $k$-fold cross validation for the example problem in [Training a Surrogate Model](/examples/surrogate_training.md).

### Normal parameter distributions

Next, we examine results from cross validation of a third degree polynomial regression for normally distributed parameters. Again, 2-fold, 5-fold, and 10-fold cross validation was repeated for 1e5 trials. The mean and standard deviation of the RMSE for each $k$ is reported in [stats_normal].

!table id=stats_normal caption=Mean and standard deviation of RMSE scores obtained for 1e5 repeated cross validation trials, for uniform parameter distributions.
| Moment          | 2-fold    | 5-fold      | 10-fold   |
| :-              | -         | -           | -         |
| $\mu_{RMSE}$    | 8.187     | 8.077       | 8.057     |
| $\sigma_{RMSE}$  | 0.1425    | 0.0486      | 0.0297    |

The cross validation results for this learning problem are significantly more pessimistic than the previous - the RMSE scores across the board are roughly 3% of $T_{max}$. The reason is straightforward - with Latin Hypercube sampling, unlikely parameter values (in the tail of the normal distribution) are sparsely represented in the sampling data. When these samples are left out for cross validation, the model is trained primarily on parameters near the mean. As a result, the polynomial model will tend to have disproportionately high error for these unlikely parameters/response pairs. This was not observed when using uniformly distributed parameters, as the full parameter range was (roughly) equally represented in all folds.

If high surrogate accuracy is needed for parameters in the tails of the probability distributions, this may indicate improvements are needed in the modeling or sampling procedure. This is a good example of a case where cross validation can be invaluable in properly assessing deficiencies in a model.

!media stochastic_tools/surrogates/cross_validation/cv_normal_1dheat.svg id=cv_normal style=width:70% caption=Distribution of RMSE reported from 10000 repetitions of 5-fold cross validation for the example problem in [Training a Surrogate Model](/examples/surrogate_training.md).

### Other surrogate model types

Cross-validation can be used to characterize differences in predictive accuracy between different types of surrogate models. For demonstration, the analysis of the preceding sections were repeated for several other Surrogate types. However, because the cost of repeated cross-validation for large datasets is more significant for some models, only 100 repetitions of cross-validation were performed with 1000 Latin Hypercube samples. This was sufficient to reveal useful differences between model types.

The following model types were used:

- Third degree [PolynomialRegression](source/trainers/PolynomialRegressionTrainer.md).
- Third degree [PolynomialChaos](source/surrogates/PolynomialChaos.md).
- [GaussianProcess](source/trainers/GaussianProcessTrainer.md), with a [SquaredExponentialCovariance](source/covariances/SquaredExponentialCovariance.md) function. Length scales for each input parameter were chosen by first performing hyperparameter tuning with a reduced training data set.
- [LibtorchANN](source/libtorch/trainers/LibtorchANNTrainer.md), with a single hidden layer and 64 neurons.
- [NearestPoint](source/trainers/NearestPointTrainer.md).

The following listing summarizes the Surrogate types considered in this comparison, along with the required control parameters.

!listing stochastic_tools/examples/surrogates/cross_validation/all_trainers_uniform_cv.i block=GlobalParams Trainers Covariance

RMSE scores for each model type were accumulated over 100 repeated trials of 5-fold cross validation, for only uniform parameter distributions. In the following table, the results are summarized by a simple mean and standard deviation of the RMSE across all trials.

!table id=all_trainers_uniform caption=Mean and standard deviation of RMSE scores for all model types, obtained for 100 repeated cross validation trials with uniform parameter distributions.
| Moment          | Polynomial Regression  | Polynomial Chaos     | Nearest Point | Gaussian Process   | Libtorch ANN |
| :-              | -         | -           | -         | -           | -         |
| $\mu_{RMSE}$    | 0.278     | 30.727      | 3.724     | 0.207       | 0.189     |
| $\sigma_{RMSE}$  | 0.004   | 1.141     | 0.066    |  0.007     | 0.019    |

[!ref](all_trainers_uniform) summarizes the model comparison results with uniform parameter distributions. An immediately striking observation is that the RMSE for the Polynomial Chaos model was two orders of magnitude greater (roughly 10% of the mean $T_{max}$) than that observed with several of the other models. This is expected, as Polynomial Chaos is known to be poor for single-predictor evaluations, and is primarily used to provide an effective means to characterize statistical moments of a response (see [PolynomialChaos](source/surrogates/PolynomialChaos.md)). Otherwise, NearestPoint has significantly greater validation error compared to the other model types. This is also expected, because NearestPoint (a piecewise constant model) is generally a poor approximation.

For the more sophisticated model types, the mean RMSE across the trial set was comparable and low, indicating that any of these models would be similarly effective as a surrogate for this problem. However, it is important to note that the Libtorch neural network showed greater variability in validation error than either the Polynomial Regression or Gaussian Process models for this problem - $\left( \frac{\sigma}{\mu} \right)_{ANN} \approx 0.1$, compared to $\left( \frac{\sigma}{\mu} \right)_{PR} \approx 0.01$ and $\left( \frac{\sigma}{\mu} \right)_{GP} \approx 0.03$. This indicates that the neural network was more sensitive to variations in the training set than these other models. This could be caused by several factors, such as overfitting, and may indicate a need to better tune the parameters used to define the model.
