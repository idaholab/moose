# IntegralPreservingFunctionIC

!syntax description /ICs/IntegralPreservingFunctionIC

## Description

Sets an initial condition while preserving an integral.
The [!param](/ICs/IntegralPreservingFunctionIC/function) is used to specify the overall "form" of the initial condition,
such that the applied initial condition is

\begin{equation}
\label{eq:ic1}
u(\vec{r}, t_0) = q_0 f(\vec{r}, t_0)
\end{equation}

where $u$ is the variable, $f$ is the function, and $q_0$ is a scaling
factor used to preserve a total [!param](/ICs/IntegralPreservingFunctionIC/magnitude) upon volume integration:

\begin{equation}
\label{eq:ic2}
q_0=\frac{Q}{\int_\Omega fd\Omega}
\end{equation}

where $Q$ is the total magnitude and $\Omega$ is the domain of integration.

## Example Input Syntax

As an example, below we set a sinusoidal heat source with generic form
$\sin{\left(\frac{\pi z}{H}\right)}$ for a total magnitude of 550 (upon volume
integration). This means that the actual initial condition is
$q_0\sin{\left(\frac{\pi z}{H}\right)}$, where $q_0$ is determined in order to
satisfy the specified total volume integral.

!listing test/tests/ics/integral_preserving_function_ic/sinusoidal_z.i
  start=ICs
  end=Executioner

!syntax parameters /ICs/IntegralPreservingFunctionIC

!syntax inputs /ICs/IntegralPreservingFunctionIC

!syntax children /ICs/IntegralPreservingFunctionIC
