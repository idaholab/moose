# AverageVariableChange

This post-processor computes a volume-weighted norm of the change of a variable
or auxiliary variable over a time step or a nonlinear iteration:

!equation
||\delta u||_{\Omega,p} = \left(\frac{1}{|\Omega|} \int\limits_\Omega |\delta u|^p \, d\Omega\right)^{\frac{1}{p}} \,,

where

- $\delta u$ is the variable change of interest,
- $\Omega$ is the domain over which the norm is taken,
- $|\Omega|$ is the volume of the domain, and
- $p$ is the order of the norm (either 1 or 2).

Denoting the current time index by $n$ and the current nonlinear iteration index
by $(\ell)$, the variable change may be with respect to either the old time value,

!equation
\delta u \equiv u_n^{(\ell)} - u_{n-1}

or the previous nonlinear iteration value,

!equation
\delta u \equiv u_n^{(\ell)} - u_n^{(\ell-1)}

as determined by the value of [!param](/Postprocessors/AverageVariableChange/change_over).

Volume-weighting the norm produces a value that is independent of the size of
the domain. Without doing this, there is a domain volume bias in the norm.
Suppose $\delta u$ is constant over the domain with a value $\overline{\delta u}$:

!equation
||\delta u||_{p} = \left(\int\limits_\Omega |\overline{\delta u}|^p \, d\Omega\right)^{\frac{1}{p}}
  = |\overline{\delta u}| |\Omega|^{\frac{1}{p}} \,.

!syntax parameters /Postprocessors/AverageVariableChange

!syntax inputs /Postprocessors/AverageVariableChange

!syntax children /Postprocessors/AverageVariableChange
