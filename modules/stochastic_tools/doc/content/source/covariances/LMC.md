# LMC

!syntax description /Covariance/LMC

The linear model of co-regionalization (LMC) distinctly models the covariances
between the $N$ inputs and the $M$ outputs. Mathematically, the LMC is defined as
[!citep](Liu2018gp,Cheng2020gp):

\begin{equation}
    \label{eqn:mogp_3}
    \bar{\pmb{K}} = \sum_{q=1}^Q \bar{\pmb{B}}_q \otimes \pmb{K}_q
\end{equation}

where, $q$ denotes the latent basis index, $\bar{\pmb{B}}_q$ output covariance
matrix of size $M \times M$ for the $q$-th covariate, $\pmb{K}_q$ is
the input covariance matrix of size $N \times N$ for the $q$-th
covariate, $Q$ is the total number of basis functions, and $\otimes$ denotes
the Kronecker product. $\bar{\pmb{B}}_q$ is further defined as the sum of
two matrices of weights [!citep](Cheng2020gp):

\begin{equation}
    \label{eqn:mogp_4}
    \bar{\pmb{B}}_q = \pmb{A}_q \pmb{A}_q^\intercal + \textrm{diag}\Big(\pmb{\lambda}_q\Big)
\end{equation}

where, $\pmb{A}_q$ and $\pmb{\lambda}_q$ are vectors (size $M\times 1$)
of hyper-parameters, both for the $q$-th basis.
The size $Q$ is user-defined and it can be greater than or equal to 1. The
larger the $Q$, the more sophisticated the multi-output Gaussian Process in
modeling complex outputs.

If $Q=1$, the LMC reduces to the intrinsic co-regionalization model (ICM).

## Example Input File Syntax

!listing test/tests/surrogates/multioutput_gp/mogp_lmc.i block=Covariance

!syntax parameters /Covariance/LMC

!syntax inputs /Covariance/LMC

!syntax children /Covariance/LMC
