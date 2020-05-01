# ACInterfaceCleavageFracture

!syntax description /Kernels/ACInterfaceCleavageFracture

Implements the Allen-Cahn term with crack propagation preferred along weak
cleavage plane specified using cleavage plane normal ($\boldsymbol{M}$). Planes
not normal to $\boldsymbol{M}$ are penalized using $\beta$ parameter
[!cite](ClaytonKnap2015). Setting $\beta=0$ results in isotropic damage with
respect to cleavage planes. The added term is

\begin{equation}
\dfrac{\beta l_0}{2} \left( \boldsymbol{I} - \boldsymbol{M} \otimes \boldsymbol{M} \right) : \left( \nabla d \otimes \nabla d \right)
\end{equation}

Its weak form is

\begin{equation}
\int_{\Omega} \left( - \beta l_0 \left( \nabla^2 d - \boldsymbol{M} \otimes \boldsymbol{M} : \nabla \nabla d \right) \right) \psi dV
\end{equation}

The second term in above expression can be simplified as,

\begin{equation}
\begin{aligned}
\int_{\Omega} \psi \left(\boldsymbol{M} \otimes \boldsymbol{M} : \nabla \nabla d \right) dV
&= \int_{\Omega} \left( \nabla \cdot \left( \psi \boldsymbol{M} \left( \boldsymbol{M} \cdot \nabla d \right) \right) - \left( \boldsymbol{M} \cdot \nabla\psi \right) \left( \boldsymbol{M} \cdot \nabla d \right) - \left( \psi \nabla \cdot \boldsymbol{M} \right) \left( \boldsymbol{M} \cdot \nabla d \right) \right) dV
\\
&= \int_{\Omega} \nabla \cdot \left( \psi \boldsymbol{M} \left( \boldsymbol{M} \cdot \nabla d \right) \right) dV - \int_{\Omega} \left( \boldsymbol{M} \cdot \nabla \psi \right) \left( \boldsymbol{M} \cdot \nabla d \right) dV - \int_{\Omega} \left( \psi \nabla \cdot \boldsymbol{M} \right) \left( \boldsymbol{M} \cdot \nabla d \right) dV
\\
&= \int_{\partial \Omega} \psi \boldsymbol{M} \left( \boldsymbol{M} \cdot \nabla d \right) \cdot \boldsymbol{n} dS - \int_{\Omega} \left( \boldsymbol{M} \cdot \nabla \psi \right) \left( \boldsymbol{M} \cdot \nabla d \right) dV - \int_{\Omega} \left( \psi \nabla \cdot \boldsymbol{M} \right) \left( \boldsymbol{M} \cdot \nabla d \right) dV
\end{aligned}
\end{equation}

In the last expression, the first and last term are zero and thus remains only
middle term.

The Jacobian is

\begin{equation}
\beta l_0 \left( \int_{\Omega} \left( \nabla \psi \cdot \nabla \phi_j \right) dV - \int_{\Omega} \left( \boldsymbol{M} \cdot \nabla \psi \right) \left( \boldsymbol{M} \cdot \nabla \phi_j \right) dV \right)
\end{equation}


!syntax parameters /Kernels/ACInterfaceCleavageFracture

!syntax inputs /Kernels/ACInterfaceCleavageFracture

!syntax children /Kernels/ACInterfaceCleavageFracture
