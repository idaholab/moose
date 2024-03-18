# IntrinsicCoregionalizationModel

!syntax description /OutputCovariance/IntrinsicCoregionalizationModel

The linear model of co-regionalization (LMC) distinctly models the covariances between the $N$ inputs and the $M$ outputs. Mathematically, the LMC is defined as [!citep](Liu2018gp,Cheng2020gp):

\begin{equation}
    \label{eqn:mogp_3}
    \bar{\pmb{K}} = \sum_{q=1}^Q \bar{\pmb{B}}_q \otimes \pmb{K}_q
\end{equation}

where, $q$ denotes the basis index, $\bar{\pmb{B}}_q$ output covariance matrix of size $M \times M$ for the $q^{\textrm{th}}$ covariate, $\pmb{K}_q$ is the inputs covariance matrix of size $N \times N$ for the $q^{\textrm{th}}$ covariate, $Q$ is the total number of basis, and $\otimes$ denotes the Kronecker product. $\bar{\pmb{B}}_q$ is further defined as the sum of two matrices of weights [!citep](Cheng2020gp):

\begin{equation}
    \label{eqn:mogp_4}
    \bar{\pmb{B}}_q = \pmb{A}_q \pmb{A}_q^\intercal + \textrm{diag}\Big(\pmb{\lambda}_q\Big)
\end{equation}

where, $\pmb{A}_q$ and $\pmb{\lambda}_q$ are the matrix (size $M\times R$) and vector (size $M\times 1$) of hyper-parameters, both for the $q^{\textrm{th}}$ basis. The size $R$ is user-defined and it can greater than or equal to 1. The larger the $R$, the more sophisticated the Multi-Output Gaussian Process in modeling complex outputs. Specifically, $\pmb{A}_q$ is defined as:

\begin{equation}
    \label{eqn:mogp_5}
    \pmb{A}_q = \begin{bmatrix}
a_{q,11} & \dots & a_{q,1R}\\
\vdots & \ddots & \vdots \\
a_{q,M1} & \dots & a_{q,MR}\\
\end{bmatrix}
\end{equation}

In addition, the size of $Q$ can also be greater than or equal to 1. If $Q=1$, the LMC reduces to the intrinsic co-regionalization model (ICM).

!alert note title=Complexity of the ICM covariance
Currently, the number of columns of matrix $\pmb{A}_q$ is restricted to $R=1$. This limitation will be removed in the future by treating $R$ as an user-defined input.

## Example Input File Syntax

!listing test/tests/surrogates/multioutputGP/mogp.i block=OutputCovariance

!syntax parameters /OutputCovariance/IntrinsicCoregionalizationModel

!syntax inputs /OutputCovariance/IntrinsicCoregionalizationModel

!syntax children /OutputCovariance/IntrinsicCoregionalizationModel