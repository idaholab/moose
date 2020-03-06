# ACInterfaceBetaPenalty

!syntax description /Kernels/ACInterfaceBetaPenalty

Implements the Allen-Cahn term with crack propagation preferred along weak
cleavage plane specified using cleavage plane normal ($n$). Planes not normal
to $n$ are penalized using $\beta$ parameter [!cite](ClaytonKnap2015). Setting
$\beta=0$ results in isotropic damage with respect to cleavage planes. The
added term is

\begin{equation}
\dfrac{\beta l_0}{2} \left( \boldsymbol{I} - \boldsymbol{n} \otimes \boldsymbol{n} \right) : \left( \nabla d \otimes \nabla d \right),
\end{equation}

Its weak form is

\begin{equation}
\left( - \beta l_0 \left( \nabla^2 d - \nabla d \otimes \nabla d : \nabla \nabla d \right) \right) \psi,
\end{equation}

and the Jacobian is

\begin{equation}
\beta l_0 \left( \nabla \psi \cdot \nabla \phi_j - \left( \boldsymbol{n} \cdot \nabla \psi \right) \left( \boldsymbol{n} \cdot \nabla \phi_j \right)
\end{equation}


!syntax parameters /Kernels/ACInterfaceBetaPenalty

!syntax inputs /Kernels/ACInterfaceBetaPenalty

!syntax children /Kernels/ACInterfaceBetaPenalty
