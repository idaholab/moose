# Polynomial Chaos Surrogate

This example explains how to train and utilize a [PolynomialChaos](PolynomialChaos.md) surrogate model. For detailed information on the polynomial chaos method including the mathematics and implementation see [PolynomialChaos](PolynomialChaos.md) and [QuadratureSampler](QuadratureSampler.md). For general information on training and evaluating a surrogate model see [Training a surrogate model](/examples/surrogate_training.md) and [Evaluating a surrogate model](/examples/surrogate_evaluate.md).

## Overview

Polynomial chaos (PC) is a stochastic element method. Its formulation is similar to finite element method, but instead of discretizing spatial dimensions, PC discretizes random variables, i.e. uncertain parameters. Within the training phase, PC creates a functional of a quantity of interest (QoI) that is dependent on the random variables. This function consists of a finite sum of orthogonal polynomials with coefficients, these coefficients are what the training phase is attempting to compute. Another difference from finite elements is that these polynomials do not necessarily have compact support and increasing the terms in the sum increases the polynomial order. Please see [PolynomialChaos.md] for more details of the polynomial chaos method.

!include surrogate_training.md start=heat_conduction_model_begin end=heat_conduction_model_finish

## Training

This section describes how to set up an input for a [PolynomialChaosTrainer](PolynomialChaosTrainer.md). There are three aspects that need to be known before setting up the input file.

1. A sub app needs to be built, defining the physics of the problem, this example uses a [heat conduction model](examples/surrogates/sub.i) described in the previous section.
2. Uncertain parameters need to be identified with a defined probability distribution.
3. Quantities of interest need to be defined, for now, these come in the form of postprocessors from the sub app.

!include surrogate_training.md start=omitting_solve_begin end=omitting_solve_finish replace=['nearest_point_training', 'poly_chaos_uniform_mc']

### Parameter Uncertainty Distributions

A required aspect of the polynomial chaos method is a definition of the probability distribution of the uncertain parameters. These distributions define the type of polynomial used for the PC expansion. This example shows the two types of distributions available for PC training:

!listing examples/surrogates/poly_chaos_uniform_mc.i block=Distributions caption=Uniform distribution for each parameter

!listing examples/surrogates/poly_chaos_normal_mc.i block=Distributions caption=Normal distribution for each parameter

### Training Sampler

Non-intrusive polynomial chaos training relies on sampling the full-order model (i.e. sub app) at a number of sample points. There are two general types of sampling techniques for PC: Monte Carlo and numerical quadrature.

#### Monte Carlo Sampling

Monte Carlo sampling is the most intuitive type of sampling and can be done using the [MonteCarlo](MonteCarloSampler.md) or [LatinHypercube](LatinHypercubeSampler.md) samplers in the stochastic tools module.

!listing examples/surrogates/poly_chaos_uniform_mc.i block=Samplers

#### Quadrature Sampling

Because PC expansion uses very specific types of polynomials, using numerical quadrature is typically a more precise approach to training a PC model. The [QuadratureSampler](QuadratureSampler.md) implements numerical quadrature for uniform and normal distributions. There are three different techniques for building a multidimensional quadrature grid: Cartesian grid, Smolyak sparse grid, and Clenshaw-Curtis sparse grid. These grids are specified by the [!param](/Samplers/Quadrature/sparse_grid) parameter: `none`, `smolyak`, and `clenshaw-curtis`. Using sparse grids can significantly reduce the number of training points while retaining accuracy, see [QuadratureSampler](QuadratureSampler.md) for more details. The number of sample points is ultimately defined by the [!param](/Samplers/Quadrature/order) parameter. It is generally advisable that this parameter be set to the same value as the [!param](/Trainers/PolynomialChaosTrainer/order) parameter in [PolynomialChaosTrainer](PolynomialChaosTrainer.md).

!listing examples/surrogates/poly_chaos_uniform_quad.i block=Samplers

### Running Sub Application

Running the sub app and transferring data back and forth for PC is exactly the same as any other type of surrogate. See [Training a surrogate model](/examples/surrogate_training.md) on more details on setting up this part of the input file.

!listing examples/surrogates/poly_chaos_uniform_quad.i block=MultiApps Controls Transfers Reporters


### Trainers

The PC training object is defined within the [Trainers](Trainers/index.md) block. The required parameters for a PC trainer are:

- [!param](/Trainers/PolynomialChaosTrainer/distributions) specify the type of polynomials to use for the expansion, it is very import that these distributions match the distributions given to the sampler.
- [!param](/Trainers/PolynomialChaosTrainer/sampler) is the object that will provide sample points which are given to the sub app during execution.
- [!param](/Trainers/PolynomialChaosTrainer/response) specifies the result vector for storing the computed values.
- [!param](/Trainers/PolynomialChaosTrainer/order) defines the maximum order of the PC expansion, this parameter ultimately defines the accuracy and complexity of the surrogate model.

!listing examples/surrogates/poly_chaos_uniform_mc.i block=Trainers

### Outputting Training Data

Outputting the data after training a PC trainer is exactly the same as outputting data for any other type of trainer. See [Training a surrogate model](/examples/surrogate_training.md) on more details on setting up this part of the input file.

!listing examples/surrogates/poly_chaos_uniform_mc.i block=Outputs

### Example Input Files

- [examples/surrogates/poly_chaos_uniform_mc.i]
- [examples/surrogates/poly_chaos_uniform_quad.i]
- [examples/surrogates/poly_chaos_normal_mc.i]
- [examples/surrogates/poly_chaos_normal_quad.i]

## Evaluation and Statistics

This section will go over how to set up an input file to evaluate and perform statistical analysis on a trained [polynomial chaos surrogate](PolynomialChaos.md). The polynomial chaos surrogate is unique in that it is able to compute statistical moments and sensitivities analytically. Specific postprocessors are available to compute these quantities without the need for sampling.

!include surrogate_training.md start=omitting_solve_begin end=omitting_solve_finish replace=['nearest_point_training', 'poly_chaos_uniform']

### Evaluation

Evaluating a polynomial chaos surrogate model is exactly the same as any other type of surrogate, see [Evaluating a surrogate model](/examples/surrogate_evaluate.md) for more details.

!listing examples/surrogates/poly_chaos_uniform.i block=Surrogates caption=Loading training data

!listing examples/surrogates/poly_chaos_uniform.i block=Distributions Samplers caption=Defining sampler for evaluation -- Uniform distributions

!listing examples/surrogates/poly_chaos_normal.i block=Distributions Samplers caption=Defining sampler for evaluation -- Normal distributions

!listing examples/surrogates/poly_chaos_uniform.i block=samp caption=Evaluating surrogate with [EvaluateSurrogate](EvaluateSurrogate.C)

### Statistics and Sensitivities

[PolynomialChaos](PolynomialChaos.md) has the ability to compute statistical moments, local sensitivity, and sobol indices analytically. These are computed using [PolynomialChaosReporter](PolynomialChaosReporter.md). The implemented moments include mean, standard deviation, skewness, and kurtosis. The type of moment is specified by the [!param](/Reporters/PolynomialChaosReporter/statistics) parameter, which can be a combination of `mean`, `stddev`, `skewness`, and `kurtosis`. This example computes mean and standard deviation. It should be noted that computing skewness and kurtosis can be very computationally demanding using the analytical technique. Therefore, it might be better to sample the surrogate model and compute these quantities with the result.

The points to compute the sensitivity can be defined explicitly by specifying the [!param](/Reporters/PolynomialChaosReporter/local_sensitivity_points) parameter, and/or using a sampler by specifying the [!param](/Reporters/PolynomialChaosReporter/local_sensitivity_sampler). It is vital that the columns of the points match with the distributions defined from the original trainer.

Sobol statistics are a metric of the global sensitivity of each parameter, or a combination of parameters, see [SobolReporter.md] for more details. These are computed by setting the [!param](/Reporters/PolynomialChaosReporter/include_sobol) parameter to `true`. This will compute total, first-, and second-order Sobol indicies.

!listing examples/surrogates/poly_chaos_uniform.i block=stats

### Example Input Files

- [examples/surrogates/poly_chaos_uniform.i]
- [examples/surrogates/poly_chaos_normal.i]

## Results and Analysis

In this section, the results of training and evaluation of the surrogate model as described in the previous sections is shown. A convergence study of the polynomial chaos training, including analysis of sampling and polynomial order, is also provided.

### Evaluation Results

[samp_uniform] and [samp_normal] show the resulting probability distributions from sampling each surrogate with 100,000 points.

!media poly_chaos_uniform_hist.svg id=samp_uniform caption=Temperature distributions with uniform parameter distribution

!media poly_chaos_normal_hist.svg id=samp_normal caption=Temperature distribution with normal parameter distribution

### Local Sensitivity Results

[local_sense] shows the results of computing the local sensitivity of each parameter for every surrogate. For reference, the the sensitivity is defined as

!equation
S_p = \frac{\partial u(\vec{\xi})}{\partial \xi_p} \frac{\xi_p}{u(\vec{\xi})}

where $u$ is either $\bar{T}$ or $T_{\max}$, $\xi_p$ is $k$, $q$, $L$, or $T_{\infty}$, and the point tested ($\vec{\xi}$) is $[k=5,q=10^4,L=0.03,T_{\infty}=300]$

!table id=local_sense caption=Local sensitivity results
| QoI | Distribution | $k$ Sensitivity | $q$ Sensitivity | $L$ Sensitivity | $T_{\infty}$ Sensitivity |
| :- | :- | - | - | - | - |
| $\bar{T}$  | Uniform | -0.001944 | 0.001985 | 0.003976 | 0.9977 |
| $\bar{T}$  | Normal  | -0.002009 | 0.001667 | 0.003303 | 0.9983 |
| $T_{\max}$ | Uniform | -0.002918 | 0.002979 | 0.005966 | 0.9967 |
| $T_{\max}$ | Normal  | -0.002897 | 0.002736 | 0.005437 | 0.9973 |

### Sobol Statistics Results

[sobol_stats] shows the results of computing Sobol statistics for average temperature and uniform parameter distributions (the other surrogates produced very similar statistics). In the plot, the x axis represents the first index subscript and the y axis represents the second. As can be seen, the statistics are symmetric, i.e. it does not matter the order of the index subscripts.

!plot scatter id=sobol_stats caption=Average temperature Sobol statistics results uniform parameter distribution
  data=[{
        'type': 'heatmap',
        'z': [[-1.92374355e+00, -4.40086571e+00, -2.25831206e+00, -1.78394694e+01],
              [-4.40086571e+00, -4.12327151e+00, -4.45782994e+00, -1.80335784e+01],
              [-2.25831206e+00, -4.45782994e+00, -1.98071681e+00, -1.81868220e+01],
              [-1.78394694e+01, -1.80335784e+01, -1.81868220e+01, -1.23598907e-02]],
        'y': ['k','q','L','T<sub>&#8734;</sub>'],
        'x': ['k','q','L','T<sub>&#8734;</sub>'],
        'colorbar': {'tickvals': [-16,-13,-10,-7,-4,-1],
                     'ticktext': ['10<sup>-16</sup>', '10<sup>-13</sup>', '10<sup>-10</sup>', '10<sup>-7</sup>', '10<sup>-4</sup>', '10<sup>-1</sup>']}
        }]

### Sampling Analysis

Here the effect of various sampling techniques on the training of a polynomial chaos surrogate is shown. Three different types of samplers were used: Monte Carlo, tensor quadrature, and Smolyak sparse quadrature. Monte Carlo was run with increasing number of samples and the quadrature was run with increasing order. Note, the polynomial expansion order was set constant. The performance metric is the error in the mean and standard deviation versus the number of total training points. [samp_avg_uniform]-[samp_max_normal] show the results of this study for each quantity of interest and each parameter distribution. The uniform distributions ([samp_avg_uniform] and [samp_max_uniform]) have an analytical mean and standard deviation, which is why there is nice convergence. However, the normal distributions ([samp_avg_normal] and [samp_max_normal]) use numerical integration to compute the reference mean and standard deviation, which is why there is a more spurious convergence at low errors.

!plot scatter id=samp_avg_uniform caption=Sample convergence for moments of average temperature with uniform parameter distributions
  filename=examples/surrogates/gold/poly_chaos_avg_uniform_results.csv
  data=[{'x':'mc_points', 'y':'mc_mu', 'name':'MC &#956; error'},
        {'x':'mc_points', 'y':'mc_sig', 'name':'MC &#963; error'},
        {'x':'tensor_points', 'y':'tensor_mu', 'name':'Tensor &#956; error'},
        {'x':'tensor_points', 'y':'tensor_sig', 'name':'Tensor &#963; error'},
        {'x':'smolyak_points', 'y':'smolyak_mu', 'name':'Smolyak &#956; error'},
        {'x':'smolyak_points', 'y':'smolyak_sig', 'name':'Smolyak &#963; error'}]
  layout={'xaxis':{'type':'log','title':'Number of Training Points'},
          'yaxis':{'type':'log','title':'Relative Moment Error'}}

!plot scatter id=samp_max_uniform caption=Sample convergence for moments of maximum temperature with uniform parameter distributions
  filename=examples/surrogates/gold/poly_chaos_max_uniform_results.csv
  data=[{'x':'mc_points', 'y':'mc_mu', 'name':'MC &#956; error'},
        {'x':'mc_points', 'y':'mc_sig', 'name':'MC &#963; error'},
        {'x':'tensor_points', 'y':'tensor_mu', 'name':'Tensor &#956; error'},
        {'x':'tensor_points', 'y':'tensor_sig', 'name':'Tensor &#963; error'},
        {'x':'smolyak_points', 'y':'smolyak_mu', 'name':'Smolyak &#956; error'},
        {'x':'smolyak_points', 'y':'smolyak_sig', 'name':'Smolyak &#963; error'}]
  layout={'xaxis':{'type':'log','title':'Number of Training Points'},
          'yaxis':{'type':'log','title':'Relative Moment Error'}}

!plot scatter id=samp_avg_normal caption=Sample convergence for moments of average temperature with normal parameter distributions
  filename=examples/surrogates/gold/poly_chaos_avg_normal_results.csv
  data=[{'x':'mc_points', 'y':'mc_mu', 'name':'MC &#956; error'},
        {'x':'mc_points', 'y':'mc_sig', 'name':'MC &#963; error'},
        {'x':'tensor_points', 'y':'tensor_mu', 'name':'Tensor &#956; error'},
        {'x':'tensor_points', 'y':'tensor_sig', 'name':'Tensor &#963; error'},
        {'x':'smolyak_points', 'y':'smolyak_mu', 'name':'Smolyak &#956; error'},
        {'x':'smolyak_points', 'y':'smolyak_sig', 'name':'Smolyak &#963; error'}]
  layout={'xaxis':{'type':'log','title':'Number of Training Points'},
          'yaxis':{'type':'log','title':'Relative Moment Error'}}

!plot scatter id=samp_max_normal caption=Sample convergence for moments of maximum temperature with normal parameter distributions
  filename=examples/surrogates/gold/poly_chaos_max_normal_results.csv
  data=[{'x':'mc_points', 'y':'mc_mu', 'name':'MC &#956; error'},
        {'x':'mc_points', 'y':'mc_sig', 'name':'MC &#963; error'},
        {'x':'tensor_points', 'y':'tensor_mu', 'name':'Tensor &#956; error'},
        {'x':'tensor_points', 'y':'tensor_sig', 'name':'Tensor &#963; error'},
        {'x':'smolyak_points', 'y':'smolyak_mu', 'name':'Smolyak &#956; error'},
        {'x':'smolyak_points', 'y':'smolyak_sig', 'name':'Smolyak &#963; error'}]
  layout={'xaxis':{'type':'log','title':'Number of Training Points'},
          'yaxis':{'type':'log','title':'Relative Moment Error'}}

### Polynomial Order Analysis

The effect of polynomial expansion order on the accuracy of the surrogate model is shown here. Again, the performance metric is the error in the mean and standard deviation versus the number of total training points. The maximum polynomial order is ranged from 3 to 9, where the tensor quadrature sampler order matches (which is generally advisable in any case). [ord_uniform] shows the result of the convergence study with a uniform distribution. Normal distribution was omitted since there is no analytical reference.

!plot scatter id=ord_uniform caption=Polynomial order convergence for moments of maximum and average temperature with uniform parameter distributions
  filename=examples/surrogates/gold/poly_chaos_order_uniform_results.csv
  data=[{'x':'tensor_points', 'y':'tensor_mu_avg', 'name':'T&#773; &#956; error'},
        {'x':'tensor_points', 'y':'tensor_sig_avg', 'name':'T&#773; &#963; error'},
        {'x':'tensor_points', 'y':'tensor_mu_max', 'name':'T<sub>max</sub> &#956; error'},
        {'x':'tensor_points', 'y':'tensor_sig_max', 'name':'T<sub>max</sub> &#963; error'}]
  layout={'xaxis':{'type':'log','title':'Number of Training Points'},
          'yaxis':{'type':'log','title':'Relative Moment Error'}}
