# MultiOutputGaussianProcessSurrogate

To understand how multi-output Gaussian processes (MOGPs) function, [/trainers/MultiOutputGaussianProcessTrainer.md] will be a good place to start. Here, only the evaluation of MOGPs will be disucssed after finishing the training.

## MOGPs forward prediction and uncertainty quantification

Once the MOGP hyper-parameters are optimized, probabilistic predictions of the vector quantities of interest can be made. Given a prediction input $\pmb{x}_*$, the probability distribution of the vector outputs is given by:

\begin{equation}
    \label{eqn:mogp_pred1}
    p(\hat{\pmb{y}}_*|\pmb{x}_*,\hat{\pmb{y}},\bar{\pmb{x}},\pmb{\theta}) = \mathcal{N}(\hat{\pmb{\mu}}_*,~\bar{\pmb{\Sigma}}_{*})
\end{equation}

where, $\bar{\pmb{x}}$ is the matrix of training inputs, $\hat{\pmb{\mu}}_*$ is the mean vector, and $\bar{\pmb{\Sigma}}_{*}$ is the covariance matrix. The mean vector is defined as:

\begin{equation}
    \label{eqn:mogp_pred2}
    \hat{\pmb{\mu}}_* = \bar{\pmb{K}}_{\hat{\pmb{y}}_*,\hat{\pmb{y}}} ~(\bar{\pmb{K}}_{\hat{\pmb{y}},\hat{\pmb{y}}})^{-1}~\hat{\pmb{y}}
\end{equation}

where, $\bar{\pmb{K}}_{\hat{\pmb{y}}_*,\hat{\pmb{y}}}$ is the full covariance matrix of the training inputs and prediction inputs and $\bar{\pmb{K}}_{\hat{\pmb{y}},\hat{\pmb{y}}}$ is the full covariance matrix of the training inputs. The covariance matrix $\bar{\pmb{\Sigma}}_{*}$ is defined as:

\begin{equation}
    \label{eqn:mogp_pred3}
    \bar{\pmb{\Sigma}}_{*} = \bar{\pmb{K}}_{\hat{\pmb{y}}_*,\hat{\pmb{y}}_*} - \bar{\pmb{K}}_{\hat{\pmb{y}}_*,\hat{\pmb{y}}}~(\bar{\pmb{K}}_{\hat{\pmb{y}},\hat{\pmb{y}}})^{-1}~\bar{\pmb{K}}_{\hat{\pmb{y}}_*,\hat{\pmb{y}}}^\intercal
\end{equation}

where, $\bar{\pmb{K}}_{\hat{\pmb{y}}_*,\hat{\pmb{y}}_*}$ is the full covariance matrix of the prediction inputs.

## Implementation in the STM

To facilitate the evaluation of MOGPs, `evaluate()` function which can broadcast vector outputs is provided.

!listing surrogates/MultiOutputGaussianProcessSurrogate.C start=MultiOutputGaussianProcessSurrogate::evaluate( end={

!alert note title=Output format for MOGPs
Vector of MOGPs means and standard deviations are output to a JSON file. Both normalized means and standard deviations followed by the un-normalized ones are output.

!syntax parameters /Surrogates/MultiOutputGaussianProcessSurrogate

!syntax inputs /Surrogates/MultiOutputGaussianProcessSurrogate

!syntax children /Surrogates/MultiOutputGaussianProcessSurrogate
