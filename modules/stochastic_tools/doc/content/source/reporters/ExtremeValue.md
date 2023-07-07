# ExtremeValue

!syntax description /Likelihood/ExtremeValue

## Overview

The ExtremeValue (Gumbel) likelihood function is given by:

\begin{equation}
    \label{eqn:triso_likelihood}
    \begin{aligned}
        &\mathcal{L}(\pmb{\theta}, \sigma | \Theta_i, \mathcal{M}, \mathcal{D}_i) = \frac{1}{\beta} \exp{\big(-(z+\exp{(-z)})\big)}\\
        &\text{where,}~z = \frac{M(\pmb{\Theta}_i) - \hat{M}(\pmb{\theta},~\Theta_i)}{\beta}\\
    \end{aligned}
\end{equation}

where, $\hat{M}(\pmb{\theta},~\pmb{\Theta}_i)$ is the model prediction given model parameters $\pmb{\theta}$ and the $i^{\text{th}}$ experimental configuration $\pmb{\Theta}_i$ and $M(\pmb{\Theta}_i)$ is the $i^{\text{th}}$ experimental data point. $\beta$ above the scale of the distribution representing the model inadequacy and experimental noise uncertainties.

!syntax parameters /Likelihood/ExtremeValue

!syntax inputs /Likelihood/ExtremeValue

!syntax children /Likelihood/ExtremeValue
