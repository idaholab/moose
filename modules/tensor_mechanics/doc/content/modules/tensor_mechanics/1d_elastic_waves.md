# Frequency Domain Dynamics

The following example presents a frequency domain analysis done in the MOOSE Tensor Mechanics module. A frequency domain analysis provides the structural response at a discrete set of frequencies. At each frequency, an independent steady state simulation is performed. This document provides an example of modeling a dynamic problem at a single frequency (time-harmonic problem).

Frequency domain analysis is often used to determine a frequency response function (FRF). An FRF describes the relationship between an input (frequency and amplitude of the input forcing source) and output (displacement response of a system). For simple systems, an analytic FRF can be derived. For more complex systems, the FRF is numerically obtained by determining the system response over a range of frequencies. The frequencies correspond to the peaks in the response on the FRF plot indicate natural frequencies of the system (eigen/fundumental frequencies). The mode shape (eigenvector) is given by the displacement profile at a natural frequency.

Other applications of frequency domain dynamics are: (1) computation of band structure (dispersion curves) of lattices/metamaterial, (2) inverse design for vibration control, e.g. design a system so that it has as minimum/maximum response at particular frequency, (3) material properties inversion/optimization given discrete responses.

On the other hand, frequency domain analyses can be advantageous over time domain counterpart in several cases, for example, when the frequency spectrum of a signal consists of a few frequencies, or, when it is needed to have a better control of the numerical dispersion.

# Problem Description

A uniaxial beam, of length L=1 see [cantilever] subjected to time-harmonic displacement on right and has absobing boundary condition on left.

!media media/tensor_mechanics/Cantilever_beam.png style=width:60%; caption=2D cantilever problem with a prescribed displacement boundary condition on the right end. id=cantilever

The PDE that mathematically describes this time-harmonic structural dynamics problem is as follows:
\begin{equation}
    -\nabla \cdot \sigma +\rho \frac{\partial^2 u}{\partial t^2}=0 \label{eq1}
\end{equation}

For the boundary condition, we apply the sommerfeld radiation condition on the left, and a harmonic source (cosine) on the right as follows:
\begin{equation}
    \sqrt E \, \nabla u= i\sqrt \rho \, \frac{\partial u}{\partial t} \, \qquad \text{at} \, x=0,
    \\
    u= A \, cos(2 \pi f t) \, \qquad \text{at} \, x=L,\label{eq2}
\end{equation}
where $f$ is the frequency, $A=0.5$, and  $2 \pi f=\omega=10$.

In [eq1], we consider Young's modulus and the density as constants and equal to unity.
[eq1] can be written as:
\begin{equation}
     -\nabla \cdot \sigma + \frac{\partial^2 u}{\partial t^2} = 0 \label{eq3}
\end{equation}

Now, we consider the plane wave $u= r e^{i(\omega t - k x)}$ is a solution for [eq3], with $r$ being the complex amplitudes, and $k$ is the wave number and $i=\sqrt{-1}$. As we noted before Young's modulus and the density are constants and equal to unity, thus, $c_p=1$ where $c_p=\sqrt \frac{E}{\rho}$ is the longitudinal wave velocity .

!alert note
One can verify that $r e^{i(\omega t - k x)}$ is a solution for [eq3]$\Longleftrightarrow$ $k^2= \frac{\omega^2}{c_p^2}$ by direct substitution.

Now, [eq2] and the associated BCs. and can be written as,
\begin{equation}
     -\nabla \cdot \sigma- \omega^2 u = 0 \label{eq4}
\end{equation}

\begin{equation}
    \nabla u= ik \, u \, \qquad \text{at} \, x=0,
    \\
    u= A \, cos(2 \pi f t) \, \qquad \text{at} \, x=L,\label{eq5}
\end{equation}

Transforming [eq4] and the associated boundary conditions in [eq5] in time to the frequency domain, we obtain:
\begin{equation}
     -\nabla \cdot \hat \sigma- \omega^2  U = 0 \label{eq6}
\end{equation}
with the following boundary conditions:
\begin{equation}
    \nabla U= ik \, U \, \qquad \text{at} \, x=0,
    \\
U= 0.5 \, \qquad \text{at} \, x=L,\label{eq7}
\end{equation}

!alert note
$u$ is a function of time and space $(x,t)$, whereas $U$ is function of space and frequency $(x,\omega)$.

where $\hat \sigma = \nabla U$ with $U(x,\omega)$ being the Fourier transform of $u(x,t)$.

!alert note title=Fourier Transfrom of Boundary Conditions
Not only the pde is transformed to the frequency domain, but also, we must transform all B.Cs as well. Transfroming $A cos(2\pi f t)$ into the frquency domain will result in an impulse at frequency = $f$ with amplitude of A.

When transform the time dependent variable $u$ to the frequency domain, the time-dependency will be omitted. The result is a nother variable $U$ that is frequency dependent. In general, $U$ is complex-valued function/variable in the form of $U_r+iU_i$, where $U_r$ and $U_i$ are the real and the imaginary part of $U$.

At this stage, we split the complex sytem of equations into two $\textbf{real}$ systems, which yields two real-valued systems that live on the same mesh. The strong form PDEs becomes as follows:

\begin{equation}
    -\nabla \cdot \hat \sigma_r- \omega^2  U_r = 0
    \\
    -\nabla \cdot \hat \sigma_i- \omega^2  U_i = 0 \label{eq8}
\end{equation}
with the following boundary conditions:
\begin{equation}
   \nabla U_r= -k U_i \quad ,\nabla U_i= k U_r\, \qquad \text{at} \, x=0,\\
   U_r=U_i=0 \, \qquad \text{at} \, x=L, \label{eq9}
\end{equation}

Note that this decomposition is exact and there is no infromation missed while decompisition. Also note that, as a result of the radiation BCs on left, $U_r$ and $U_i$ are coupled, hence the two systems in [eq8].

The following is how we define the kernels and the BCs. of this problem,

!listing examples/wave_propagation/1D_elastic_wave_propagation.i block=Kernels id=listing1

 and here is how we define the associated boundary conditions.

!listing examples/wave_propagation/1D_elastic_wave_propagation.i block=BCs id=listing2

!media media/tensor_mechanics/wavefieldandprofile.png style=width:40%; caption=the solution at $\omega =10$. id=solution

The analytical solution of [eq6] and the associated boundary conditions in [eq7] is obtained by expressing the solution of $U$ as a combination of sines and cosines representing the traveling and reflecting waves, i.e $U=C_1 e^{ikx} + C_2e^{-ikx}$.

Applying the boundary conditions, and considering the wave to travel in the negative $x$ direction, we obtain, $C_1=0$ with $C_2=0.5/(e^{-ikL})$. So, the analytical solution is: $U=0.5 e^{ik(L-x)}.$
The moose solution for this problem is shown in [solution].
