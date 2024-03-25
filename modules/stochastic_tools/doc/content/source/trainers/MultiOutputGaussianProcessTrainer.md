# MultiOutputGaussianProcessTrainer

Before understanding multi-output Gaussian processes (MOGPs), it would help understanding scalar GPs theory. Please refer to [/trainers/GaussianProcessTrainer.md] for an introduction to scalar GPs. Here, a brief review of scalar GPs will be provided followed by an explanation of MOGPs.

## Scalar GPs

GPs approximate the function $f(\pmb{x})$ using a Gaussian distribution in the form of:

\begin{equation}
    \label{eqn:gp_1}
    \begin{aligned}
    &f(\pmb{x}) = \mathcal{N}\Big(m(\pmb{x}), \kappa(\pmb{x},\pmb{x}^\prime)\Big)\\
    &m(\pmb{x}) = \mathbb{E}[f(\pmb{x})]\\
    &\kappa(\pmb{x},\pmb{x}^\prime) = \mathbb{E}\Big[\Big(f(\pmb{x})-m(\pmb{x})\Big)~\Big(f(\pmb{x}^\prime)-m(\pmb{x}^\prime)\Big)\Big]\\
    \end{aligned}
\end{equation}

\noindent where, $\pmb{x}$ is the input vector, $m(\pmb{x})$ is the mathematical expectation of $f(\pmb{x})$, and $\kappa(\pmb{x},\pmb{x}^\prime)$ is the kernel function that defines the covariance between $\pmb{x}$ and a neighboring location $\pmb{x}^\prime$. The function $f(\pmb{x})$ can be the scalar quantity of interest from a computational model or observed experimental data. If we have $N$ scalar observations, they jointly follow a Gaussian distribution defined as [!citep](rasmussen2005gaussian, dhulipala2022al):

\begin{equation}
    \label{eqn:gp_2}
    \begin{aligned}
    &\pmb{y} = [f(\pmb{x}_1),~f(\pmb{x}_2),\dots,~f(\pmb{x}_N)]^\intercal \sim \mathcal{N}(\pmb{\mu},~\pmb{K})\\
    &\pmb{\mu} = \Big[\mathbb{E}[f(\pmb{x}_1)],~\mathbb{E}[f(\pmb{x}_2)],\dots,~\mathbb{E}[f(\pmb{x}_N)]\Big]^\intercal \\
    &\pmb{K}_{ij} = \kappa(\pmb{x}_i,\pmb{x}_j)\\
    \end{aligned}
\end{equation}

\noindent where, $\pmb{\mu}$ is the mean vector and $\pmb{K}$ is the covariance matrix. An element $\pmb{K}_{ij}$ of $\pmb{K}$ defines the covariance function or kernel function between $\pmb{x}_i$ and $\pmb{x}_j$. A popular kernel function used for setting up the covariance matrix $\pmb{K}$ is the squared exponential kernel:

\begin{equation}
    \label{eqn:gp_3}
    \kappa(\pmb{x},\pmb{x}^\prime) = \sigma^2~\exp \Bigg(-\frac{1}{2}\sum_{d=1}^D\frac{(x_d-{x}_{d}^\prime)^2}{l_d^2}\Bigg)
\end{equation}

\noindent, where $\sigma^2$ is the amplitude parameter and $l_d$ is the length scale for input dimension $d$, and $D$ is the number of input dimensions. Herein, we will use GPs and MOGPs to model the response of a computational model whose data is not corrupted by random noise. As such, the noise variance term which is commonly modeled using GPs and MOGPs, is ignored. Overall, a GP with the squared exponential kernel requires the specification of $D+1$ hyper-parameters; that is, $D$ length scales and the amplitude parameter. For complex problems, these hyper-parameters are usually obtained by optimizing the log-likelihood function. In a GP, owing to the Gaussianity, the log-likelihood function has a closed form expression as shown below:

\begin{equation}
    \label{eqn:gp_4}
    \mathcal{L} = -\frac{1}{2}~\ln |\pmb{K}| - \frac{1}{2}~\pmb{y}^T~\pmb{K}^{-1}~\pmb{y}-\frac{1}{2}~N~\ln(2\pi)
\end{equation}

\noindent The log-likelihood function is constructed from the training data of model outputs vector $\pmb{y}$ and the inputs $[\pmb{x}_1,\pmb{x}_2,\dots,~\pmb{x}_N]$.

## MOGPs

MOGPs model and predict the outputs which are vectors, each of size $M$. For any input vector $\pmb{x}_i$, the vector of outputs $\pmb{y}_i$ and the matrix of $N$ vectors $\bar{\pmb{Y}}$ is defined as:

\begin{equation}
    \label{eqn:mogp_1}
    \begin{aligned}
    &\pmb{y}_i = [f(\pmb{x}_i)^1,~f(\pmb{x}_i)^2,\dots,~f(\pmb{x}_i)^M]^\intercal \\
    &\bar{\pmb{Y}} = [\pmb{y}_1,~\pmb{y}_2,\dots,~\pmb{y}_N]^\intercal \\
    \end{aligned}
\end{equation}

\noindent where, $\pmb{y}_i$ is of size $M\times 1$ and $\bar{\pmb{Y}}$ is of size $N\times M$. The matrix $\bar{\pmb{Y}}$ is vectorized and represented as $\hat{\pmb{y}}$ with size $NM \times 1$. $\hat{\pmb{y}}$ is modeled as a Gaussian distribution defined as:

\begin{equation}
    \label{eqn:mogp_2}
    \hat{\pmb{y}} \sim \mathcal{N}\Big(\hat{\pmb{\mu}},~\bar{\pmb{K}}\Big)
\end{equation}

\noindent where, $\hat{\pmb{\mu}}$ is the mean vector and $\bar{\pmb{K}}$ is the full covariance matrix. $\bar{\pmb{K}}$ captures covariances across the input variables and the vector of outputs and hence has size $NM \times NM$. $\bar{\pmb{K}}$ can be modeled in several ways as discussed in [!citep](Liu2018gp,Alvarez2012gp). We will follow the linear model of co-regionalization (LMC) which distinctly models the covariances between the $N$ inputs and the $M$ outputs. Mathematically, the LMC is defined as [!citep](Liu2018gp,Cheng2020gp):

\begin{equation}
    \label{eqn:mogp_3}
    \bar{\pmb{K}} = \sum_{q=1}^Q \bar{\pmb{B}}_q \otimes \pmb{K}_q
\end{equation}

\noindent where, $q$ denotes the basis index, $\bar{\pmb{B}}_q$ output covariance matrix of size $M \times M$ for the $q^{\textrm{th}}$ covariate, $\pmb{K}_q$ is the inputs covariance matrix of size $N \times N$ for the $q^{\textrm{th}}$ covariate, $Q$ is the total number of basis, and $\otimes$ denotes the Kronecker product. $\bar{\pmb{B}}_q$ is further defined as the sum of two matrices of weights [!citep](Cheng2020gp):

\begin{equation}
    \label{eqn:mogp_4}
    \bar{\pmb{B}}_q = \pmb{A}_q \pmb{A}_q^\intercal + \textrm{diag}\Big(\pmb{\lambda}_q\Big)
\end{equation}

\noindent where, $\pmb{A}_q$ and $\pmb{\lambda}_q$ are the matrix (size $M\times R$) and vector (size $M\times 1$) of hyper-parameters, both for the $q^{\textrm{th}}$ basis. The size $R$ is user-defined and it can greater than or equal to 1. The larger the $R$, the more sophisticated the MOGP in modeling complex outputs. Specifically, $\pmb{A}_q$ is defined as:

\begin{equation}
    \label{eqn:mogp_5}
    \pmb{A}_q = \begin{bmatrix}
a_{q,11} & \dots & a_{q,1R}\\
\vdots & \ddots & \vdots \\
a_{q,M1} & \dots & a_{q,MR}\\
\end{bmatrix}
\end{equation}

!table caption=A summary of the vectors and matrices used in MOGPs with their sizes. $N$ is the number of inputs  and $M$ is the number of outputs for each input.
| Notation | Size | Description |
| - | - | - |
| $\pmb{y}_i$ | $M\times 1$ | $i^{\textrm{th}}$ output vector |
| $\bar{\pmb{Y}}$ | $N\times M$ | Output matrix |
| $\hat{\pmb{y}}$ | $N M \times 1$ | Output vector |
| $\hat{\pmb{\mu}}$ | $NM\times 1$ | Mean vector |
| $\bar{\pmb{K}}$ | $N M\times N M$ | Full covariance matrix |
| $\bar{\pmb{B}}_q$ | $M\times M$ | Output covariance matrix |
| $\pmb{K}_q$ | $N\times N$ | Input covariance matrix |
| $\pmb{A}_q$ | $M\times R$ | Weight matrix for $q^{\textrm{th}}$ basis |
| $\pmb{\lambda}_q$ | $M\times 1$ | Intra-covariate weights for $q^{\textrm{th}}$ basis |

## MOGP hyper-parameter optimization

The hyper-parameters of the MOGP model are inferred by optimizing the log-likelihood function. The MOGP log-likelihood function has a form similar to a scalar GP:

\begin{equation}
    \label{eqn:mogp_opt1}
    \mathcal{L} = -\frac{1}{2}~\ln |\bar{\pmb{K}}| - \frac{1}{2}~\hat{\pmb{y}}^T~\bar{\pmb{K}}^{-1}~\hat{\pmb{y}}-\frac{1}{2}~N~\ln(2\pi)
\end{equation}

We will use the adaptive moment estimation (Adam) algorithm to optimize the MOGP hyper-parameters \cite{kingma2014adam}. Adam is a stochastic optimization algorithm that permits mini-batch sampling during the optimization iterations. The optimization of MOGPs can be expensive. If there are $N$ training points each with $M$ outputs, each training iteration of Adam has a cost of $\mathcal{O}(M^3N^3)$. Adam permits using $n < N$ random training points during each iteration which has a cost of $\mathcal{O}(M^3n^3)<<\mathcal{O}(M^3N^3)$.

!syntax parameters /Trainers/MultiOutputGaussianProcessTrainer

!syntax inputs /Trainers/MultiOutputGaussianProcessTrainer

!syntax children /Trainers/MultiOutputGaussianProcessTrainer