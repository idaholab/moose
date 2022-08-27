# GaussianProcessTrainer

"Gaussian Processes for Machine Learning" [!cite](rasmussen2005gaussian) provides a well written discussion of Gaussian Processes, and its reading is highly encouraged. Chapters 1-5 cover the topics presented here with far greater detail, depth, and rigor.

The documentation here is meant to give some practical insight for users to begin creating surrogate models with Gaussian Processes.

Given a set of inputs $X=\lbrace{\vec{x}_1, \cdots, \vec{x}_m \rbrace}$ for which we have made observations of the correspond outputs $Y=\lbrace{\vec{y}_1, \cdots, \vec{y}_m \rbrace}$ using the system ($Y = f(X)$). Given another set of inputs $X_\star=\lbrace{\vec{x}_{\star 1}, \cdots, \vec{x}_{\star n} \rbrace}$ we wish to predict the associated outputs $Y_\star=f(X_\star)$ without evaluation of $f(X_\star)$, which is presumed costly.

#### Parameter Covariance

In overly simplistic terms, Gaussian Process Modeling is driven by the idea that trials which are "close" in their input parameter space will be "close" in their output space. Closeness in the parameter space is driven by the covariance function $k(\vec{x},\vec{x'})$ (also called a kernel function, not to be confused with a MOOSE Framework kernel). This covariance function is used to generate a covariance matrix between the complete set of parameters $X \cup X_\star = \lbrace{\vec{x}_1, \cdots, \vec{x}_m, \vec{x}_{\star 1}, \cdots, \vec{x}_{\star n} \rbrace}$, which can then be interpreted block-wise as various covariance matrices between $X$ and $X_\star$.

!equation
\begin{aligned}
\mathbf{K}(X \cup X_\star,X \cup X_\star) & = \left[
\begin{array}{ccc|ccc}
k(\vec{x}_1,\vec{x}_1) & \cdots & k(\vec{x}_1,\vec{x}_m)  & k(\vec{x}_{1},\vec{x}_{\star 1}) & \cdots & k(\vec{x}_{1},\vec{x}_{\star n}) \\
\vdots &   & \vdots  & \vdots &   & \vdots \\
(\vec{x}_m,\vec{x}_1)  & \cdots & k(\vec{x}_m,\vec{x}_m) & k(\vec{x}_{m},\vec{x}_{\star 1})  & \cdots & k(\vec{x}_{m},\vec{x}_{\star n}) \\ \hline
k(\vec{x}_{\star 1},\vec{x}_{1}) & \cdots & k(\vec{x}_{\star 1},\vec{x}_{m}) &  k(\vec{x}_{\star 1},\vec{x}_{\star 1}) & \cdots & k(\vec{x}_{\star 1},\vec{x}_{\star n}) \\
\vdots &   & \vdots & \vdots &   & \vdots \\
k(\vec{x}_{\star n},\vec{x}_{1})  & \cdots & k(\vec{x}_{\star n},\vec{x}_{m}) & k(\vec{x}_{\star n},\vec{x}_{\star 1})  & \cdots & k(\vec{x}_{\star n},\vec{x}_{\star n})
\end{array}
\right] \\
& =\left[
\begin{array}{c|c}
 \mathbf{K}(X,X) & \mathbf{K}(X,X_\star) \\ \hline
  \mathbf{K}(X_\star,X) & \mathbf{K}(X_\star,X_\star)
\end{array}
\right] \\
& =\left[
\begin{array}{c|c}
 \mathbf{K} & \mathbf{K}_\star \\ \hline
  \mathbf{K}_\star^T  & \mathbf{K}_{\star \star}
\end{array}
\right]
\end{aligned}

The Gaussian Process Model consists of an infinite collection of functions, all of which agree with the training/observation data. Importantly the collection has closed forms for 2nd order statistics (mean and variance). When used as a surrogate, the nominal value is chosen to be the mean value. The method can be broken down into two step: definition of the prior distribution then conditioning on observed data.

#### Gaussian processes

A Gaussian Process is a (potentially infinite) collection of random variables, such that the joint distribution of every finite selection of random variables from the collection is a Gaussian distribution.

!equation
\mathcal{GP}(\mu(\vec{x}),k(\vec{x},\vec{x'}))  

In an analogous way that a multivariate Gaussian is completely defined by its mean vector and its covariance matrix, a Gaussian Process is completely defined by its mean function and covariance function.

The (potentially) infinite number of random variables within the Gaussian Process correspond to the (potentially) infinite points in the parameter space our surrogate can be evaluated at.

#### Prior distribution:

We assume the observations (both training and testing) are pulled from an $m+n$ multivariate Gaussian distribution. The covariance matrix $\Sigma$ is the result of the choice of covariance function.

!equation
Y \cup Y_\star \sim \mathcal{N}(\mu,\Sigma)

Note that $\mu$ and $\Sigma$ are a vector and matrix respectively, and are a result of the mean and covariance functions applied to the sample points.  

+Zero Mean Assumption:+ Discussions of Gaussian Process are typically presented under assumption that $\mu=0$. This occurs without loss of generality since any sample can be made $\mu=0$ by subtracting the sample mean (or a variety of other preprocessing options). Note that in a training\testing paradigm, the testing data $Y_\star$ is unknown, so determination of what to use as $\mu$ is based on the information from the training data $Y$ (or some other prior assumption).

#### Conditioning:

With the prior formed as above, conditioning on the available training data $Y$ is performed. This alters the mean and variance to new values $\mu_\star$ and $\Sigma_\star$, restricting the set of possible functions which agree with the training data.

!equation
\begin{aligned}
\mu_\star &= \mu + \mathbf{K}_\star \mathbf{K}^{-1}(Y-\mu) \\
\Sigma_\star &= \mathbf{K}_{\star \star} - \mathbf{K}_\star^T \mathbf{K}^{-1} \mathbf{K}_\star
\end{aligned}

!equation
Y_\star \sim \mathcal{N}(\mu_\star ,\Sigma_\star)

When used as a surrogate, the nominal value is typically taken as the mean value, with $diag(\Sigma_\star)$ providing variances which can be used to generate confidence intervals.

## Common Hyperparameters

While the only apparent decision in the above formulation is the choice of covariance function, most covariance functions will contain hyperparameters of some form which need to be selected in some manner. While each covariance function will have its own set of hyperparameters, a few hyperparameters of specific forms are present in many common covariance functions.



#### Length Factor $\ell$ or $\vec{\ell}$

Frequently Kernels consider the distance between two input parameters $\vec{x}$ and $\vec{x}^\prime$. For system of only a single parameter this distance often takes the form of

!equation
\frac{|x - x^\prime|}{\ell}.

In this form the $\ell$ factor set a relevant length scale for the distance measurements.

When multiple input parameters are to be considered, it may be advantageous to specify $n$ different length scales for each of the $n$ parameters, resulting in a vector $\vec{\ell}$. For example distance may be calculated as

!equation
\sqrt{ \sum_{i=1}^n \left( \frac{x_i - x^\prime_i}{\ell_i} \right)^2}.

When used with standardized parameters, $\ell$ can be interpreted in units of standard deviation for the relevant parameter.

#### Signal Variance $\sigma_f^2$

This serves as an overall scaling parameter. Given a covariance function $\tilde{k}$ (which is not a function of $\sigma_f^2$), the multiplication of $\sigma_f^2$ yields a new valid covariance function.

!equation
k(x,x^\prime,\sigma_f) = \sigma_f^2 \, \tilde{k}(x,x^\prime)

This multiplication can also be pulled out of the covariance matrix formation, and simply multiply the matrix formed by $\tilde{k}$

!equation
\mathbf{K}(x,x^\prime,\sigma_f) = \sigma_f^2 \, \tilde{\mathbf{K}}(x,x^\prime)


#### Noise Variance $\sigma_n^2$

The $\sigma_n^2$ represents noise in the collected data, and is as a additional $\sigma_n^2$ factor on the variance terms (when $x=x^\prime$).

!equation
k(x,x^\prime,\sigma_f, \sigma_n) = \sigma_f^2 \, \tilde{k}(x,x^\prime) + \sigma_n^2 \, \delta_{x,x^\prime}

In the matrix representation this adds a factor of $\sigma_n^2$ to diagonal of the noiseless matrix $\tilde{\mathbf{K}}$

!equation
\mathbf{K}(x,x^\prime,\sigma_f, \sigma_n) = \sigma_f^2 \, \tilde{\mathbf{K}}(x,x^\prime) + \sigma_n^2 \mathbf{I}

Due to the addition of $\sigma_n^2$ along the diagonal of the $K$ matrix, this hyperparameter can aid in the the inversion of the covariance matrix. For this reason adding a small amount of $\sigma_n^2$ may be preferable, even when you believe the data to be noise free.


## Selected Covariance Functions

!table id=HyperpramTable caption=Selected Covariance Functions
| Covariance Function | Description |
| - | - |
| [](SquaredExponentialCovariance.md) | Also referred to as a radial basis function (RBF) this is a widely used, general purpose covariance function. Serves as a common starting point for many. |
| [](ExponentialCovariance.md) | A simple exponential covariance function. |
| [](MaternHalfIntCovariance.md) | Implementation of the Matern class of covariance function, where the $\nu$ parameter takes on half-integer values. |

## Hyperparameter tuning options

The following options are available for tuning the hyperparameters:

- +PETSc/TAO+

  Relies on the TAO optimization library from PETSc. Several optimization algorithms are available from this library. Note that these algorithms perform deterministic optimization.

- +Adaptive moment estimation (Adam)+

  Relies on the psuedocode provied in [!cite](kingma2014adam). Adam permits stochastic optimization, wherein, a batch of the training data can be randomly chosen at each iteration.

!syntax parameters /Trainers/GaussianProcessTrainer

!syntax inputs /Trainers/GaussianProcessTrainer

!syntax children /Trainers/GaussianProcessTrainer
