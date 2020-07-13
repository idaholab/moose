# ADACKappaFunction

!syntax description /Kernels/ADACKappaFunction

This kernel is the AD version of [ACKappaFunction](\ACKappaFunction). It adds the following terms to the phase field equations

\begin{equation}
 \frac{1}{2} \frac{\partial\kappa}{\partial\eta_i}
  |\nabla\eta_i|^2
\end{equation}

where $\eta_i$ is the nonlinear variable, $\kappa$ is the gradient energy coefficients. When `kappa` is a function of the phase field variables, this kernel should be used
to calculate the term which includes the derivatives of kappa.


!syntax parameters /Kernels/ADACKappaFunction

!syntax inputs /Kernels/ADACKappaFunction

!syntax children /Kernels/ADACKappaFunction
