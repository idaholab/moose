# Frequency Domain Analysis

The following example presents two frequency domain analysis of a cantilever beam done in the MOOSE Tensor Mechanics module. The first example computes a frequency response function of the cantilever beam and identifies the first two eigenvalues of beam.  The second analysis computes the dynamic response at a single frequency (time-harmonic problem).
A frequency domain analysis provides the structural response at a discrete set of frequencies. At each frequency, an independent steady state simulation is performed. This document provides an example of modeling a dynamic problem at a single frequency (time-harmonic problem).

Frequency domain analysis is often used to determine a frequency response function (FRF). An FRF describes the relationship between an input (frequency and amplitude of the input forcing source) and output (displacement response of a system). For simple systems, an analytic FRF can be derived. For more complex systems, the FRF is numerically obtained by determining the system response over a range of frequencies. The frequencies corresponding to the peaks on the FRF plot indicate natural frequencies of the system (eigen/fundumental frequencies). The mode shape (eigenvector) is given by the displacement profile at a natural frequency.

Other applications of frequency domain dynamics are: (1) computation of band structure (dispersion curves) of lattices/metamaterial, (2) inverse design for vibration control, e.g. design a system so that it has as minimum/maximum response at particular frequency, (3) material properties inversion/optimization given discrete responses.

Frequency domain analyses can be advantageous over its time domain counterpart in several cases, for example, when the frequency spectrum of a signal consists of a few frequencies, or, when it is needed to have a better control of the numerical dispersion.

# Problem Description

The equations of motion for a one dimensional isotropic elastic solid is given by the following partial differential equation:
\begin{equation}
    -E\frac{\partial^2 u}{\partial x^2}+\rho \frac{\partial^2 u}{\partial t^2}=0
\label{eq1}
\end{equation}
where $E$ is the modulus of elasticity, and $\rho$ is the density.  
To convert to the frequency domain, we consider that a plane wave given by
\begin{equation}
  u(x,t)= B e^{i(\omega t - k x)}
  \label{eq2}
\end{equation}
is a solution to [eq1] where $B$ is in general a complex number depending on the boundary conditions, $k^2=\omega^2\rho/E$ is the wave number, $\omega=2 \pi f$ where $f$ is the frequency, and $i=\sqrt{-1}$.  
By assuming [eq2] is a a solution to [eq1] we can solve [eq1] in the frequency domain by taking a Fourier transform of $u(x,t)$ to get $U(x,\omega)$.  The frequency domain version of [eq1] is the Helmholtz equation given by
\begin{equation}
    E\frac{\partial^2 U}{\partial x^2}-\rho\omega^2 U=0.
\label{eq3}
\end{equation}
[eq3] is easily solved in MOOSE where $U$ is the state variable.  The first term on the right hand side is still captured by the [StressDivergence.md] kernel.  The second RHS term, $\rho\omega^2 U$, is captured by the [Reaction](/Reaction.md) kernel where the `Reaction` rate is given by $-\omega^2$.  [eq3] is only valid for small displacements and linear elasticity.  A damping term can also be included in [eq1] and [eq3].

The boundary conditions also need to be converted to the frequency domain through a Fourier transform.  A time harmonic Neumann BC given by
\begin{equation}
    f= A \, cos(2 \pi f t) \qquad \text{at} \, x=L
    \label{eq4}
\end{equation}
with amplitude $A$ and frequency $f$, is transformed to
\begin{equation}
    F= A \, \qquad \text{at} \, x=L
    \label{eq5}
\end{equation}

# Cantilever Beam Example

The cantilever beam shown in [cantilever] is subjected to a time harmonic force on the right side in the out-of-plane and vertical directions.  In this example the frequency of the time varying load is swept over a range.  The displacement at the midpoint of the beam is recorded at each frequency.  This type of output, displacement as a function of frequency, is a frequency response function (FRF) or transfer function.  Frequencies corresponding to the displacement peaks in the FRF indicate natural frequencies/modes.

!media media/tensor_mechanics/Cantilever_beam.png style=width:60%; caption=2D cantilever problem with a prescribed displacement boundary condition on the right end. id=cantilever

The analytic solution for the free vibration of a cantilever [Euler Bernoulli beam](https://en.wikipedia.org/wiki/Euler%E2%80%93Bernoulli_beam_theory).  The analytic eigenvalues, $\omega_n$, are given by
\begin{equation}
  \omega_n=k^2_n\sqrt\frac{EI^2}{\rho A L^4}
\end{equation}
where $I$ is the moment of inertia, $A$ the cross sectional area, $L$ the beam length, and $k_n$ are the wave numbers.
For a cantilever beam, the first three wave numbers, $k_n$, given for the Euler Bernoulli beam dimensions given as the dimensions of the cantilever beam are $L=$1m with cross section dimensions $a=$0.1m and $b=$0.15m are\\
$k_1=1.875$\\
$k_2=4.694$\\
$k_3=7.855$\\
The moment of inertia for a rectangular cross section beam is $I=\frac{wh^3}{12}$, where $h$ is the dimension in the direction being loaded and $w$ is the other cross sectional dimension.  

For an aluminum cantilever beam, $E=$68e9 Pa, $\nu=$0.36, $\rho=$27.e3 kg/m$^3$.  
The analytic first and second natural frequencies for this system bending in directions tangential to the beam axis are:\\
$\omega_{1a}=$509Hz\\
$\omega_{1b}=$763Hz\\
Both of these frequencies use the same value of $k_1$, but with the moment of inertia recomputed for bending about the different widths, where the lower frequency is the first bending mode about the thin direction ($h=a=$0.1m) and the next higher frequency is the first bending mode about the thicker direction ($h=b=$0.15m).
The simulated natural frequencies given by peaks in the FRF for a coarse mesh are:\\
$\omega_{1a}=$600Hz\\
$\omega_{1b}=$800Hz\\
where 50Hz frequency increments are used in the FRF frequency sweep. The FRF where each displacement is plotted separately is shown in [cantileverfrf] where each mode is excited separately.  A scaled displacement magnitude is shown in [cantileverfrfmag] for a coarse and fine mesh.  A coarse mesh shows a stiffer response and and the natural frequencies are over-estimated.  The natural frequencies converge on the analytic result from above as the mesh is refined.
The simulations will fail if they are run at the natural frequencies because the solution will become singular,
i.e the displacements blow-up as shown by the asymptopes in [cantileverfrf].

!media media/tensor_mechanics/Cantilever_frfmag.png style=width:60%; caption=Cantilever beam displacement magnitude for coarse mesh (0.1m elements) and fine mesh (0.033m elements).  Analytic results are shown in grey. id=cantileverfrfmag

!media media/tensor_mechanics/Cantilever_frf.png style=width:60%; caption=Cantilever beam y- and z-displacements for the fine mesh (0.033m elements).  Analytic results are shown in grey. id=cantileverfrf

The input file for the coarse simulation shown in [cantileverfrfmag] and [cantileverfrf] is given in [listing0].

!listing examples/wave_propagation/cantilever_sweep.i id=listing0

This uses the [TensorMechanics MasterAction](Modules/TensorMechanics/Master/index.md) to setup the StressDivergence Kernels because this simulation only includes real displacements.  A `Transient` executioner with a `ConstantDT` timestepper is used to sweep over frequencies where the time is substituted for the frequency.  The time is converted to the frequency dependent `Reaction` `rate` using a `ParsedFunction` and is applied to the `rate` using the `RealFunctionControl`.

In the next example, imaginary displacements will be introduced by including an absorbing boundary condition.  Imaginary displacement represent a phase shift in the problem and often result from damping.  

# Uniaxial Beam Example

In the Fourier domain, The displacement field $U$ is in general a complex number.  In the previous example, only the real part of the displacement field was accounted for.  In this example, an absorbing boundary condition will introduce a coupling between the real and imaginary displacement fields.  This example uses the uniaxial beam shown in [cantilever] which is subjected to time-harmonic displacement on right and has an absorbing boundary condition on left.

For the boundary condition, we apply the Sommerfeld radiation condition on the left, and a harmonic source (cosine) on the right as follows:
\begin{equation}
    \sqrt E \, \nabla u= i\sqrt \rho \, \frac{\partial u}{\partial t} \, \qquad \text{at} \, x=0
\label{eq6}
\end{equation}
The frequency domain version of [eq6] is
\begin{equation}
    \frac{\partial U}{\partial x} U= ik\sqrt\frac{\rho}{E} U \qquad \text{at} \, x=0,
    \label{eq7}
\end{equation}

As mentioned before, $U$ is complex-valued function/variable in the form of $U_r+iU_i$ where $U_r$ and $U_i$ are the real and the imaginary part of $U$.  At this stage, we split the complex system of equations into two real valued systems that live on the same mesh. [eq3] for a complex valued system becomes
\begin{equation}
    E\frac{\partial^2 U_r}{\partial x^2}- \rho\omega^2  U_r = 0
    \\
    E\frac{\partial^2 U_i}{\partial x^2}- \rho\omega^2  U_i = 0
    \label{eq8}
\end{equation}
The Sommerfeld boundary conditions are given by
\begin{equation}
   \frac{\partial U_r}{\partial x}= -k U_i\, \qquad \text{at} \, x=0\\
   \frac{\partial U_i}{\partial x}= k U_r\, \qquad \text{at} \, x=0,\\
   U_r=A \, \qquad \text{at} \, x=L,\\
   U_i=0 \, \qquad \text{at} \, x=L,
   \label{eq9}
\end{equation}
Note that this decomposition is exact and no information is lost from the decomposition into real and imaginary parts. The real and imaginary [StressDivergence.md] kernels are shown in [listing1].  Care must be taken in defining these kernels and the respective displacements for the problem.  Setting the displacements in the `[GlobalParams]` block could have unintended consequences and should be set in the individual kernels that use them.  Also note that, as a result of the radiation BCs on left, $U_r$ and $U_i$ are coupled, hence the two systems in [eq8] are needed.  The Sommerfeld radiation BCs are applied using a [CoupledVarNeumannBC.md] in [listing2].

!listing examples/wave_propagation/1D_elastic_wave_propagation.i block=Kernels id=listing1 caption=Real and imaginary `StressDivergence` and `Reaction` Kernels.

!listing examples/wave_propagation/1D_elastic_wave_propagation.i block=BCs id=listing2 caption=Real and imaginary BCs.

For this example given in [listing1] and [listing2], the Young's modulus and the density are constants equal to unity.  The amplitude and frequency of the harmonic displacement BC in [eq9] are $A=5$ and $\omega=10$.

The analytical solution of [eq8] and the associated boundary conditions in [eq9] are obtained by expressing the solution of $U$ as a combination of sines and cosines representing the traveling and reflecting waves, i.e $U=C_1 e^{ikx} + C_2e^{-ikx}$.

Applying the boundary conditions, and considering the wave to travel in the negative $x$ direction, we obtain, $C_1=0$ with $C_2=0.5/(e^{-ikL})$. Plugging these values into the analytical solution gives: $U=0.5 e^{ik(L-x)}.$
The moose solution for this problem is shown in [solution].

!media media/tensor_mechanics/wavefieldandprofile.png style=width:40%; caption=Real and imaginary displacements at $\omega =10$. id=solution
