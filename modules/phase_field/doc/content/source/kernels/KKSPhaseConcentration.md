# KKSPhaseConcentration

!syntax description /Kernels/KKSPhaseConcentration

Enforces the split of the
concentration into the phase concentrations, weighted by the switching function.
The non-linear variable of this Kernel is $c_b$.

\begin{equation}
c = [1-h(\eta)]c_a+h(\eta)c_b
\end{equation}

### Residual

\begin{equation}
R=[1-h(\eta)]c_a + h(\eta)c_b - c
\end{equation}

### Jacobian

#### On-Diagonal

Since the non-linear variable is $c_b$,

\begin{equation}
J= \phi_j \frac{\partial R}{\partial c_b} = \phi_j h(\eta)
\end{equation}

#### Off-Diagonal

For $c_a$

\begin{equation}
J= \phi_j \frac{\partial R}{\partial c_a} = \phi_j [1-h(\eta)]
\end{equation}

For $c$

\begin{equation}
J= \phi_j \frac{\partial R}{\partial c} = -\phi_j
\end{equation}

For $\eta$

\begin{equation}
J= \phi_j \frac{\partial R}{\partial \eta} = \phi_j \frac{dh}{d\eta}(c_b-c_a)
\end{equation}

!syntax parameters /Kernels/KKSPhaseConcentration

!syntax inputs /Kernels/KKSPhaseConcentration

!syntax children /Kernels/KKSPhaseConcentration
