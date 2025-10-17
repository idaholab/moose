# Magnetodynamic Problem on a Simply Connected Conductor

## Summary

Solves for the magnetic field around a simply connected conductor passing through the boundary of the computational
  domain carrying a net current, using a magnetic vector potential discretized using
  $H(\mathrm{curl})$ conforming Nédélec elements.

## Description

This problem solves the low-frequency magnetodynamic problem with strong form:

\begin{equation}
\begin{split}
\vec \nabla \cdot \vec B
&= 0 \\
 \vec \nabla \times \vec H &= \vec J  \\
 \vec \nabla \times \vec E &= - \partial_t \vec B
\end{split}
\end{equation}

with the constitutive relations

\begin{equation}
\begin{split}
\vec B
&= \mu \vec H \\
 \vec E &= \rho \vec J + \vec E_{ext}\\
 \vec J &= \sigma (\vec E - \vec E_{ext})
\end{split}
\end{equation}

where $\vec B$ is the magnetic flux density, $\vec H$ is the magnetic field, $\vec J$ is the current
 density, $\vec E$ is the electric field, $\vec E_{ext}$ is the externally applied electric field,
 $\mu$ is the material permeability, $\rho$ is the material resistivity, and $\sigma$ is the material
 conductivity.


 In order to strongly enforce the divergence free condition on $\vec B$, we shall also introduce the
 magnetic vector potential $\vec A$ in the entire domain $\Omega$ and an associated scalar electric potential $\phi$ in the conducting domain $\Omega_c$, such that

\begin{equation} \begin{split} \vec B &= \vec \nabla \times \vec A \in \Omega \\
\vec E &=
  \left\{
  \begin{split}
   -\vec \nabla \phi - \partial_t \vec A &\in \Omega_c \\
   -\partial_t \vec A &\in \Omega \backslash \Omega_c
   \end{split}
   \right.
  \end{split}
\end{equation}

satisfies $\vec \nabla \cdot \vec B= 0$ and $\vec \nabla \times \vec E = - \partial_t \vec B$ by construction.

Thus, to solve our problem, we must solve $\vec \nabla \times \vec H = \vec J$ in weak form for a self-consistent pair of $\vec A$ and $\phi$. Multiplying $\vec \nabla \times \vec H = \vec J$ and $\vec \nabla \cdot \vec J = 0$ by the test functions $\partial_t  \vec A'$ and $\phi'$ respectively, and integrating over $\Omega$ with the constraint that $\vec E_{ext} =0$, we obtain the weak form

\begin{equation}
\begin{split}
(\mu^{-1} \vec \nabla \times \vec A, \vec \nabla \times \partial_t \vec A')_\Omega - (\vec H \times \vec n, \partial_t \vec A')_{\partial \Omega}
+(\sigma \partial_t \vec A, \partial_t \vec A')_{\Omega_c}
+(\sigma \vec \nabla \phi, \partial_t \vec A')_{\Omega_c}
&= 0 \,\,\, \forall \, \partial_t \vec A' \in V_A \\
 (\sigma \vec \nabla \phi, \vec \nabla \phi')_{\Omega_c}
+(\sigma \partial_t \vec A, \vec \nabla \phi')_{\Omega_c} &= -(\vec J \cdot \vec n, \phi')_{\partial \Omega} \,\,\, \forall \, \phi' \in V_\phi\end{split}
\end{equation}

where

\begin{equation}
\begin{split}
\vec A,\, \partial_t \vec A ,\, \partial_t \vec A' &\in H(\mathrm{curl})(\Omega) \\
\phi,\, \phi' &\in H^1(\Omega_c)
\end{split}
\end{equation}

## Example File

The full current-driven magnetodynamic example detailed above can be found below:

!listing test/tests/mfem/kernels/av_magnetodynamic.i
