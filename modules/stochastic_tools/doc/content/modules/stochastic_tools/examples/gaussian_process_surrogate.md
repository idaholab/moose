# Gaussian Process Surrogate

This example walks through the creation of a few gaussian process surrogates on a simple example system with an analytical solution for comparison. The first surrogate considers a single input parameter to be varied, which lends itself to a simple visual interpretation of the surrogate behavior. The next surrogate extends this idea to two input parameters being modeled. Lastly the full system is modeled with all input parameters, and compared to the analytical solution using sampling. It's recommended users be familiar with the basic surrogate framework, such as [examples/surrogate_creation.md], [examples/surrogate_training.md], and [examples/surrogate_evaluate.md].

## Problem Statement

 The full order model we wish to emulate with this surrogate is a one-dimensional heat conduction model with four input parameters $\lbrace k, q, L, T_{\infty} \rbrace$.

!equation
-k\frac{d^2T}{dx^2} = q \,, \quad x\in[0, L]

!equation
\begin{aligned}
\left.\frac{dT}{dx}\right|_{x=0} &= 0 \\
T(x=L) &= T_{\infty}
\end{aligned}

When creating a surrogate using Gaussian Processes, a quantity of interest should be chosen (as opposed to attempting to model $T(x)$ directly). In this The quantity of interest chosen for this example is the average temperature:


### Input Parameters

!table id=param_table
| Parameter | Symbol | Normal | Uniform |
| :- | - | - | - |
| Conductivity | $k$ | $\sim\mathcal{N}(5, 2)$ | $\sim\mathcal{U}(0, 20)$ |
| Volumetric Heat Source | $q$ | $\sim\mathcal{N}(10000, 500)$ | $\sim\mathcal{U}(7000, 13000)$ |
| Domain Size | $L$ | $\sim\mathcal{N}(0.03, 0.01)$ | $\sim\mathcal{U}(0, 0.1)$ |
| Right Boundary Temperature | $T_{\infty}$ | $\sim\mathcal{N}(300, 10)$ | $\sim\mathcal{U}(270, 330)$ |


### Analytical Solutions

This simple model problem has analytical descriptions for the field temperature and average temperature:

!equation
\begin{aligned}
T(x,k,q,L,T_{\infty}) &= \frac{q}{2k}\left(L^2 - x^2\right) + T_{\infty} \\
\bar{T}(k,q,L,T_{\infty}) &= \frac{qL^2}{3k} + T_{\infty} \\
\end{aligned}

For that reason $\bar{T}$ is chosen as the QoI to be modeled by the surrogate for this example problem. The closed form solution allows for easy comparison between the surrogate and the exact solution.


## Setting up a 1D Problem

Problems with a single input variable are a good place to provide insight on Gaussian Process regression. To accomplish this three parameters of our model system are fixed $\lbrace k=5, L=0.03, T_{\infty}=300 \rbrace$, leaving the surrogate to only model the action of varying $q$.

6 training values for $q$ were selected from $\sim\mathcal{U}(1, 10)$ and evaluated using a full model evaluation. The Gaussian Process model was fitted to these data points.

The Gaussian Process was chosen to use a [SquaredExponentialCovariance.md] covariance function, using three user selected hyperparameter settings: $\lbrace \sigma_n=1E-3, \sigma_f=1 , \ell_q=0.38971 \rbrace$. To set up the training for this surrogate we require the standard `Trainers` block found in all surrogate models in addition to the Gaussian Process specific `Covariance` block. Hyperparameters vary depending on the covariance function selected, and are therefore specified in the `Covariance` block.

!listing examples/surrogates/gaussian_process/gaussian_process_uniform_1D.i block=Trainers Covariance

Creation of the `Surrogate` block follows the standard procedure laid out for other surrogate models.

!listing examples/surrogates/gaussian_process/gaussian_process_uniform_1D.i block=Surrogates

One advantage of Gaussian Process surrogates is the ability to provide an model for uncertainty. To output this data the standard [EvaluateSurrogate](EvaluateSurrogate.md) reporter is used with specification of [!param](/Reporters/EvaluateSurrogate/evaluate_std), which also outputs the standard deviation of the surrogate at the evaluation point.  

!listing examples/surrogates/gaussian_process/gaussian_process_uniform_1D.i block=Reporters

The Gaussian Process surrogate model can only be evaluated at discrete points in the parameter space, so if we wish to visualize the response model fine sampling is required. To accomplish this sampling a [CartesianProductSampler.md] evaluates the model for 100 evenly spaced $q$ values in $[9000,11000]$. This sampling is plotted below in [1D_untuned] (space between the 100 sampled points are filled by simple linear interpolation, so strictly speaking the plot is not exactly the model)

!listing examples/surrogates/gaussian_process/gaussian_process_uniform_1D.i block=Samplers

!media gaussian_process_uniform_1D.svg id=1D_untuned style=width:60% caption=Surrogate model for 1D problem $\bar{T}(q)$ using fixed (user specified) hyperparameters.

[1D_untuned] demonstrates some basic principles of the gaussian process surrogate for this covariance function. Near training points (red + markers), the uncertainty in the model trends towards the measurement noise $\sigma_n$. The model function is smooth and infinitely differentiable. As we move away from the data points the model tends to just predict the mean of the training data, particularly noticeable in the extrapolation regions of the graph.

However, given that in this scenario we know the model should be a simple linear fit, we may conclude that this fit should be better. To achieve a better fit the model needs to be adjusted, specifically better hyperparameters for the covariance function likely need to be tested. For many hyperparameters this can be accomplished automatically by the system. To enable this specify the tuned hyperparameters using [!param](/Trainers/GaussianProcessTrainer/tune_parameters) in the trainer. Tuning bounds can be placed using [!param](/Trainers/GaussianProcessTrainer/tuning_min) and [!param](/Trainers/GaussianProcessTrainer/tuning_max).

!listing examples/surrogates/gaussian_process/gaussian_process_uniform_1D_tuned.i block=Trainers Covariance

To demonstrate the importance of hyperparameter tuning, the same data set was then used to train a surrogate with hyperparameter auto-tuning enabled. In this mode the system attempts to learn an optimal hyperparameter configuration to maximize the likelihood of observing the data from the fitted model. The results shown in [1D_tuned] is a nearly linear fit, with very little uncertainty in the fit, which is what we expect from the analytical form.


!media gaussian_process_uniform_1D_tuned.svg id=1D_tuned style=width:60% caption=Surrogate model for 1D problem $\bar{T}(q)$ using tuned hyperparameters.

Inspection of the final hyperparameter values after tuning gives $\lbrace \sigma_n=3.79E-6, \sigma_f=3.87 , \ell_q=4.59 \rbrace$, the significant increase in the length scale $\ell_q$ is a primary factor in the improved fit.


## Setting up a 2D Problem

Next the idea is extended to two input dimensions and attempt to model the QoI behavior due to varying $k$ and $q$, while fixing values of $\lbrace L=0.03, T_{\infty}=300 \rbrace$.

Due to the increased dimensionality, 50 training samples were selected from $q \in [9000,11000]$ and $k \in [1,10]$. An extra hyperparameter $\ell_k$ is also added to the set of hyperparameters to be tuned.  

!listing examples/surrogates/gaussian_process/gaussian_process_uniform_2D_tuned.i block=Samplers Trainers Covariance

As was done in the 1D case above, first the surrogate was fitted using fixed hyperparameters $\lbrace \sigma_n=1E-3, \sigma_f=10, \ell_q=0.38971 , \ell_k=0.38971 \rbrace$. The QoI surface is shown in [2D_untuned], with the color map corresponding to the surrogate model's uncertainty at that point. As was the case in the 1D mode, uncertainty is highest at points furthest from training data points, and the overall response deviates strongly from the expected $\frac{q}{k}$ behavior predicted.  

!media gaussian_process_uniform_2D.svg id=2D_untuned style=width:60% caption=Surrogate model for 2D problem $\bar{T}(q,k)$ using fixed hyperparameters.

Hyperparameter tuning is then enabled by setting [!param](/Trainers/GaussianProcessTrainer/tune_parameters). [2D_tuned] shows the fit with automatically tuned hyperparameters using the same data set. This results in a fit that much better captures the $\frac{q}{k}$ nature of the QoI response, with "high" uncertainty occurring primarily in extrapolation zones.

!media gaussian_process_uniform_2D_tuned.svg id=2D_tuned style=width:60% caption=Surrogate model for 2D problem $\bar{T}(q,k)$ using tuned hyperparameters.

## Full 4D Problem

This idea is then extended naturally to the full problem in which the parameter set $\lbrace k, q, L, T_{\infty} \rbrace$ is modeled. Training occurs using $500$ training points in. Evaluation occurs by sampling the surrogate $10000$ times with perturbed inputs, with results shown in histogram form in [hist]. While the $10000$ evaluation points are sampled from the normal distribution in [param_table],the $500$ training data points are sampled from the uniform distributions. Sampling the training data from the normal distribution can result in an imbalance of data points near the mean, causing poor performance in outlying regions. The training and evaluation inputs are [examples/surrogates/gaussian_process/GP_normal_mc.i] and [examples/surrogates/gaussian_process/GP_normal.i] respectively.

!media normal_hist.svg id=hist style=width:60% caption=Histogram of $10000$ samples of surrogate $\bar{T}(q,k,L,T_{\infty})$ compared to exact.
