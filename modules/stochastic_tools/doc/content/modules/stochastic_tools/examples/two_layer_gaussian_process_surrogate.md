# Two-Layer Gaussian Process Surrogate

The Two-Layer Gaussian Process (TGP) is an extension of the standard Gaussian Process (GP), often considered a simplified form of a Deep Gaussian Process (DGP). It is designed to capture more complex, non-linear, or non-stationary relationships in data by introducing a **latent variable layer** $W$ between the inputs $X$ and the response $Y$. The fundamental structure is modeled as $Y = f(W)$ and $W = g(X)$.

This hierarchical structure grants the TGP greater flexibility than a standard GP, particularly when trained within a Bayesian framework using MCMC (Markov Chain Monte Carlo) for **joint training** of hyperparameters and latent variables.

## Problem Statement

This example uses the same **1D heat conduction model** as the full-order model (FOM) introduced in the standard GP example.

* **Input Parameters**: The model focuses on two varying input parameters: $\lbrace k, q \rbrace$.
* **Quantity of Interest (QoI)**: The surrogate models the system's **average temperature** $\bar{T}$.

### Input Parameters

| Parameter | Symbol | Uniform Distribution |
| :--- | :--- | :--- |
| Conductivity | $k$ | $\sim\mathcal{U}(1, 10)$ |
| Volumetric Heat Source | $q$ | $\sim\mathcal{U}(9000, 11000)$ |

## TGP Model Setup and MCMC Training

This configuration sets up a 2D problem, modeling the two input parameters $\lbrace k, q \rbrace$, and uses the [SquaredExponentialCovariance.md] kernel with MCMC training to obtain samples of the posterior distribution.

The TGP mode is activated in the `GaussianProcessTrainer` by setting `num_layers = 2`. The configuration specifies to tune the observation layer's hyperparameters: `covar:signal_variance` and `covar:length_factor`.

!listing TGP_squared_exponential_tuned_mcmc.i block=Trainers Covariance

### TGP Training Mechanism (MCMC)

In TGP mode, training is handled by the `TwoLayerGaussianProcess` object, which is specialized for MCMC optimization. The process involves the **joint sampling** of all random variables in the model:

* **Observation Layer** Hyperparameters: $\sigma_n$ (noise), $\sigma_f$ (scale), $\ell_y$ (length scales).
* **Latent Layer** Hyperparameters: $\ell_w$ (length scales, implicitly handled).
* **Latent Variables**: $W$ (latent function values at training points).

The MCMC loop repeatedly applies Metropolis-Hastings steps to update these variables, storing a collection of posterior samples.

### Model Evaluation and Uncertainty

TGP evaluation is more complex than standard GP evaluation, as it must marginalize over the uncertainty captured by the MCMC samples. The evaluation is performed by the `TwoLayerGaussianProcessSurrogate`.

The core logic of `TwoLayerGaussianProcessSurrogate::evaluate` is based on **Bayesian Model Averaging**:

1.  **Iterative Kriging**: The function loops through the $N_{mcmc}$ stored posterior samples of $W$, $\ell_y$, and $\sigma_n$.
2.  **Two-Step Prediction**: In each MCMC step $t$, a two-step Kriging prediction is performed:
    * **Latent Prediction**: Kriging is performed on the latent variables $W$ (first layer) to predict the new latent point $\mathbf{w_{new}}$.
    * **Observation Prediction**: $\mathbf{w_{new}}$ is used as input to the observation layer (second layer) Kriging to predict the final response $Y$, yielding a mean $\mu_t$ and variance $\Sigma_t$.
3.  **Final Statistics**: The final predictive mean is the average of all sampled means ($\bar{\mu} = \frac{1}{N_{mcmc}} \sum \mu_t$). The final predictive variance $\Sigma$ combines the average Kriging variance and the variance of the sampled means:
$$
\Sigma = \underbrace{\frac{1}{N_{mcmc}} \sum_{t} \Sigma_t}_{\text{Avg Kriging Variance}} + \underbrace{\frac{1}{N_{mcmc}-1} \sum_{t} (\mu_t - \bar{\mu})^2}_{\text{Variance of Sampled Means}}
$$

