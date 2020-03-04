#ACInterfaceBetaPenalty

#Reference:
#Clayton J D and Knap J, (2015),
#Phase field modeling of directional fracture in anisotropic polycrystals,
#Computational Materials Science, Vol 98, pages 158 - 169

!syntax description /Kernels/ACInterfaceBetaPenalty

Based on the ACInterface, this kernel implements an added term for
anisotropic fracture along the cleavage plane. The added term is
\begin{
  equation
}
\dfrac{\beta l_0} { 2 }
\left( \bm{I} - \bm { n } \otimes \bm { n } \right)
    : \left( \nabla c \otimes \nabla c \right)
\end{equation}

      Its weak form is

\begin {
  equation
}
\left(- \beta l_0 \left( \nabla ^ 2 c - \nabla c \otimes \nabla c
                         : \nabla \nabla c \right) \right) \psi
\end{equation}

Simplified expression would be

\begin {
  equation
}
\beta l_0 \left( - \psi \nabla^2 c + \psi \left( \bm{
  n} \otimes \bm{n} : \nabla \nabla c \right)
\end{equation}

where $\beta$ (`beta_penalty`) is the penalty parameter to prefer crack
propagation along one cleavage plane as against other cleavage planes,
$l_0$ is the characteristic length in phase field fracture model,
$\bm{I}$ is the identity tensor,
$\bm{n}$ (`cleavage_plane_normal`) is the normal to the weak cleavage plane
$c$ is the phase field variable
and $\psi$ is the test function.

The expression for Jacobian would be

\begin{
  equation}
\beta l_0 \left( \nable \psi \cdot \nable \phi_j - \left( \bm{
  n} \cdot \nabla \psi \right) \left( \bm{
  n} \cdot \nabla \phi_j \right)
\end{
  equation}


!syntax parameters /Kernels/ACInterfaceBetaPenalty

!syntax inputs /Kernels/ACInterfaceBetaPenalty

!syntax children /Kernels/ACInterfaceBetaPenalty
