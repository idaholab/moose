#Reconstructed Discontinuous Galerkin

##Theoretical Development


###Governing Equations

The Euler equations governing unsteady compressible inviscid flows can be expressed in the conservative form as

$$
\begin{equation}
\label{eq:euler_equations}
\frac{\partial{\bf W}({\bf x},t)}{\partial t}
+ \frac{\partial{\bf F}_j({\bf W}({\bf x},t))}{\partial x_j}
= 0,
\end{equation}
$$

where the conservative state vector, $\bf W$, is defined as ${\bf W} = \left[ \rho, \rho u_i, \rho E \right]^{\tt T}$, and the conservative flux vectors, $\bf F$, are defined as ${\bf F} = \left[ \rho u_j, \rho u_i u_j, u_j (\rho E + p) \right]^{\tt T}$, where the summation convention is used. $\rho$, $p$, and $E$ denote the density, pressure, and specific total energy of the fluid, respectively; and $u_i$ is the velocity of the flow in the coordinate direction $x_i$. This set of equations is completed by the addition of the equation of state, $p = (\gamma -1)\rho(E - \frac{1}{2} u_j u_i)$, which is valid for ideal gas. $\gamma$ is the ratio of the specific heats.

###Finite Volume Methods

The above equation has been discretized in space using a cell-centered FV method. In a FV method, the computational domain $\Omega$ is divided by a set of non-overlapping control volumes $\Omega_i$, which can be one or a combination of the most common element types, e.g.,
line segment in 1D, triangles and quadrilaterals in 2D, and tetrahedra, prisms, pyramids, and hexahedra in 3D. On each control volume, the integral form of the governing equations is required to be satisfied,
$$
\begin{equation}
\label{eq:fvm_integral}
  \int_{\Omega_i}\frac{\partial{\bf W}}{\partial t}~{\rm d}V
+ \int_{\Omega_i}\nabla\cdot{\bf F}~{\rm d}V
= 0.
\end{equation}
$$
The cell-averaged conserved variable vectors, ${\bf W}_i$,
are taken to be the unknowns and defined by
${\bf W}_i(t) = \frac{1}{V_i}\int_{\Omega_i}{\bf W}({\bf x}, t)~{\rm d}V$,
where $V_i$ is the volume of the control volume $\Omega_i$.
The following equations can then be derived using the divergence theorem,
$$
\begin{equation}
  V_i\frac{{\rm d}{\bf W}_i}{{\rm d}t}
+ \sum_{j \in N_i}\int_{\Gamma_{ij}}{\bf F}_{ij}\cdot{\bf n}_{ij}~{\rm d}S
+ \sum_{\Gamma_{ib} \in \partial\Omega}\int_{\Gamma_{ib}}{\bf F}_{ib}\cdot{\bf n}_{ib}~{\rm d}S
= 0,
\label{eq:fvm_divergence_theorem_bc}
\end{equation}
$$
where $\Gamma_{ij}=\partial\Omega_i\cap\partial\Omega_j$ denotes an interior common face between cell $\Omega_i$ and $\Omega_j$, $\Gamma_{ib}=\partial\Omega_i\cap\partial\Omega$ denotes a face on the boundary of domain $\Omega$; and ${\bf n}_{ij}$ and ${\bf n}_{ib}$ are the unit vectors normal to face $\Gamma_{ij}$ and $\Gamma_{ib}$, respectively. For each cell $\Omega_i$, $N_i$ represents a set of neighboring cells, $\Omega_j$, sharing a common face, $\Gamma_{ij}$. Because the numerical solution is discontinuous between cell interfaces, the interface fluxes are not uniquely defined. The flux function, ${\bf F}_{ij}\cdot{\bf n}_{ij}$, is replaced by a numerical Riemann flux function $\mathcal{H}({\bf W}_{ij}, {\bf W}_{ji}, {\bf n}_{ij})$, where ${\bf W}_{ij}$ and ${\bf W}_{ji}$ are the conservative state vector at the left and right side of the cell interface ($i < j$). In the case of first-order FVMs, the solution in each cell is assumed to be constant in space. Then on any interior face, $\Gamma_{ij}$, the two states are simply ${\bf W}_{ij} = {\bf W}_i$ and ${\bf W}_{ji} = {\bf W}_j$. In order to guarantee consistency and conservation, $\mathcal{H}({\bf W}_{ij}, {\bf W}_{ji}, {\bf n}_{ij})$ is required to satisfy $\mathcal{H}({\bf W}_{ij}, {\bf W}_{ji}, {\bf n}_{ij}) = {\bf F}_{i}\cdot{\bf n}_{ij}$, and $\mathcal{H}({\bf W}_{ij}, {\bf W}_{ji}, {\bf n}_{ij}) = -\mathcal{H}({\bf W}_{ji}, {\bf W}_{ij}, {\bf n}_{ij})$. Similarly, the flux function on the domain boundary, ${\bf F}_{ib}\cdot{\bf n}_{ib}$, should be determined by $\mathcal{H}({\bf W}_{ib}, {\bf W}_b, {\bf n}_{ib})$ with the use of appropriate boundary conditions (BCs) satisfying the characteristic theory.

Finally, the boundary integration is approximated using one point quadrature at the midpoint of the face, and the semi-discrete form of the equations may be written as
$$
\begin{equation}
\label{eq:fvm_semi_discrete}
  V_i\frac{{\rm d}{\bf W}_i}{{\rm d}t}
+ \sum_{j \in N_i}\mathcal{H}({\bf W}_{ij}, {\bf W}_{ji}, {\bf n}_{ij})S_{ij}
+ \sum_{\Gamma_{ib} \in \partial\Omega}\mathcal{H}_b({\bf W}_{ib}, {\bf W}_b, {\bf n}_{ib})S_{ib}
= 0,
\end{equation}
$$
where $S_{ij}$ is the length of cell edge in 2D, and area of cell face in 3D.
In this work, the Riemann flux function is approximated
using the HLLC (Harten-Lax-van Leer-Contact) approximate Riemann solver \cite{batten1997average}.
This HLLC scheme is found to have the following properties:
(1) exact preservation of isolated contact and shear waves,
(2) positivity-preserving of scalar quantity, and
(3) enforcement of entropy condition.
In addition, the implementation of HLLC Riemann solver
is easier and the computational cost is lower compared with other available Riemann solvers.

By assembling all the elemental contributions,
a system of ordinary differential equations
governing the evolution of the discrete solution in time can be written as
$$
\begin{equation}
{\bf M}\frac{{\rm d}{\bf W}}{{\rm d}t} = -{\bf R}({\bf W}),
\end{equation}
$$
where $\bf M$ denotes the mass matrix,
$\bf W$ is the global vector of the degrees of freedom,
and ${\bf R}({\bf W})$ is the residual vector.
$\bf M$ has a block diagonal structure
that couples the degrees of freedom of the unknown vector associated to ${\bf W}_i$ only within $\Omega_i$.
As a result, the inverse of $\bf M$ can be easily computed in advance considering one cell at a time.

###HWENO Reconstruction

In FVMs, solution accuracy of second order can be achieved using a piecewise linear gradient reconstruction, in which a linear representation of the solution must be found in each cell, $\Omega_i$, given cell-averaged flow variables in $\Omega_i$ as well as in face-neighboring cells, $\Omega_j (j \in N_i)$. One of the most commonly used piecewise linear reconstruction schemes is the 1-exact least-squares (L-S) reconstruction \cite{barth1993recent}, where the gradients of flow variables are computed in the form of a minimization problem. This scheme is adopted in our work to calculate the in-cell gradients of flow variables. The L-S gradient reconstruction can be performed on the conservative, primitive, or characteristic flow variables, and accuracy of the reconstruction does not differ noticeably between each variable type. However, the conserved variables are not as robust as the primitive variables for ensuring the positivity of fluid density and pressure in the case of single-phase flow, although the implementation of the former is computationally cheaper. Moreover, according to \cite{gryngarten2012multi},
the conserved variables are not recommended for reconstruction in the case of multi-phase flow, as they cause severe oscillations near fluid-fluid interfaces. Therefore, to keep consistency of the present and future work, we have chosen the primitive variables in the form of ${\bf Q} = \left[ T, u_i, p \right]^{\tt T}$ in the reconstruction.

The second-order FV method described above will produce non-physical oscillations and nonlinear instability for flows with strong discontinuities. A common solution to this problem is to use a slope limiter. However, one drawback of the slope limiters is that they frequently identify regions near smooth extrema as requiring limiting and this typically results in a reduction of the optimal second-order convergence rate. In CFD applications, active limiters close to smooth extrema, such as a sharp corner or a tip of a geometric configuration, will contaminate the solution in the flow field and ultimately destroy the desired second-order accuracy of the solution. Alternatively, the ENO/WENO methods can be used as a nonlinear limiter for the FVMs as they are more robust than the slope limiters, and can achieve uniform high-order accuracy
and sharp, essentially non-oscillatory shock transition. This can be accomplished by replacing the piecewise linearly reconstructed solution polynomials with the WENO reconstructed polynomials, which maintain the original cell averages of conservative flow variables, and have 1) second-order accuracy in the regions where the solution is smooth, and 2) oscillation-free behavior in the vicinity of discontinuities.

This work is based on a WENO reconstruction scheme originally introduced by \cite{dumbser2007arbitrary,dumbser2007quadrature}. In this scheme, a linear polynomial on cell $\Omega_i$ is obtained using a nonlinear WENO reconstruction as a convex combination of the piecewise linearly reconstructed gradients at the cell itself ($k=0$) and its face-neighboring cells $(k=1, …, N_i)$,
$$
\begin{equation}
\label{eq:weno}
\nabla q^{\rm WENO}_i = \sum_{k=0}^{N_i}w_k \nabla q_k,
\end{equation}
$$
where $q_i$ is a component of the primitive variable vector, ${\bf Q}_i$, in cell $\Omega_i$, $\nabla q^{\rm WENO}_i$ is the WENO reconstructed gradient of $q_i$ in cell $\Omega_i$, $\nabla q_k$ is the piecewise linearly reconstructed gradient of $q_k$ in cell $\Omega_k$, and $w_k$ are the normalized nonlinear weights in cell $\Omega_k$. The stencils in the reconstruction are chosen only in a von Neumann neighborhood, i.e. adjacent face-neighboring cells, in order to be compact and consistent with the underlying FV method. The following figure shows a cell $\Omega_e$ of an arbitrary shape in 2D, where the following five stencils ($\Omega_e\Omega_e$, $\Omega_e\Omega_a$, $\Omega_e\Omega_b$, $\Omega_e\Omega_c$,$\Omega_e\Omega_d$) are chosen to construct a Hermite polynomial such that,
$$
\begin{equation}
\frac{1}{V_e}\int_{\Omega_e}q~{\tt d}\Omega = {\bar q_e},
\hspace{2em}
\frac{1}{V_k}\int_{\Omega_k}\nabla q^{\rm WENO}~{\rm d}\Omega = \nabla q^{\rm WENO}|_k \hspace{1em} (k = e, a, b, c, d)
\end{equation}
$$
Although the total number of stencils for each cell depends on the shape of the cell, the presented choice is unique and compact, as only the von Neumann neighbors are involved. This idea has also been extended to higher-order discontinuous Galerkin (DG) methods, where the curvatures of polynomial solutions are reconstructed in a similar fashion \cite{luo2012hermite,luo2013reconstructed}.

!media docs/media/navier_stokes/rdg/weno_stencil.png width=50% padding-right=20px caption=Neighborhood defined by von Neumann neighbors of cell $\Omega_e$ used for HWENO reconstruction on unstructured cells.

The calculation of $\nabla q^{\rm WENO}_i$ consists of the following steps:

  1. Calculation of the so-called oscillation indicator,
     $$
     \begin{equation}
     o_k = \left[\nabla q_k \cdot \nabla q_k\right]^{\frac{1}{2}},
     \end{equation}
     $$
     which is used to assess the smoothness of the piecewise linearly reconstructed solution polynomial.

  2. And finally, calculation of the non-normalized nonlinear weights,
     $$
     \begin{equation}
     \tilde{w}_k = \frac{\lambda_k}{(\varepsilon+o_i)^\gamma}.
     \label{eq:non-normalized-nonlinear-weights}
     \end{equation}
     $$
     where $\varepsilon$ is a small positive number used to avoid division by zero.
     $\tilde{w}_k$ are functions of the linear weights, $\lambda_k$, and oscillation indicator, $o_k$.
     $\lambda_k$ can be chosen to balance the accuracy and the non-oscillatory property of the FV method.
     $\gamma$ is an integer parameter to control how fast the nonlinear weights decay for non-smooth stencils.
     The set of $\lambda_k = 1$ and $\gamma = 2$ are used in this work for both smooth and non-smooth flow tests.
     This combination of parameters delivers optimal mesh convergence in smooth flow problems
     and non-oscillatory resolution of strong discontinuities in non-smooth flow problems.

  3. Calculation of the normalized nonlinear weights, $w_k$:
     $$
     \begin{equation}
     w_k = \frac{\tilde{w}_k}{\sum_{k=0}^{N_i}\tilde{w}_k}
     \end{equation}
     $$

To summarize, the resulting HWENO gradient reconstruction scheme uses the piecewise linearly reconstructed gradients on the cell itself as the central stencil, as well as the piecewise linearly reconstructed gradients on its face-neighboring cells as biased stencils. However, this scheme is not compact in a strict sense, as neighbors` neighbors are used in solution update. Similarly, the well-known minmax slope limiter by \cite{barth1989design} is not compact in this sense either. But like the minmax limiter, the stencil used in the HWENO reconstruction is compact as it involves only the von Neumann neighbors. Therefore this HWENO scheme can be implemented in a compact manner, meaning that no additional data structure is required from the underlying FV method. Notice that it is an attractive feature favored also by other unstructured mesh based frameworks \cite{christon2016hybrid,liu2016comparative}.

##Code Implementation

The second-order, cell-centered FVM described above has been implemented in the MOOSE framework \cite{gaston2009moose,gaston2015physics}. This method can be regarded as a subset of the modal DG methods using monomials. Since the framework supports the DG methods in addition to the continuous FE methods, the implementation of a stabilized, second-order FVM in the framework becomes possible.

The architecture of the resulting code has a plug-and-play modular design structure
where each piece of the residual term in a weak form of the governing PDEs is represented as a "Kernel".
Kernels may be coupled together to achieve different application goals.
All of the Kernels are required to supply a residual,
which usually involves summing products of finite element shape functions.
The basic architecture of the code allows convenient coupling of different processes and incorporation of new physics. The following figure displays a rough sketch of the basic code architecture, with the Kernels at the uppermost level, directly underlain by the framework and solver libraries used to couple the Kernels and perform the simulations. This code also enjoys an abundance of existing capabilities in the framework. For example, by inheriting the dimension agnostic design from the baseline libMesh finite element library \cite{kirk2006libmesh}, the code is allowed to run on arbitrary types of unstructured meshes in 1D, 2D, and 3D. Moreover, a large selection of explicit and implicit time integration schemes are readily available for the code too.

!media docs/media/navier_stokes/rdg/code_structure.png width=70% padding-right=20px caption=The object-oriented architecture used to develop the CFD code.

##Examples in 1D

The code has passed five benchmark test problems in 1D (in $x$):

1. Sode shock tube
2. Lax-Harten shock tube
3. double rarefaction wave
4. Woodward-Collela blast wave
5. Sedov intense point explosion

The problems listed above can be found in the navier_stokes test directory. The second problem is displayed here as an example.

###Lax-Harten Shock Tube

The Lax-Harten shock tube problem is more challenging than the Sod shock tube problem,
since the shock wave involved is much stronger in the Lax-Harten shock tube.
In this numerical experiment, $200$ cells are uniformly distributed in $x$.
A reflecting BC is imposed at the left and right boundaries.
The initial conditions at $t = 0 $ are given as:
$\rho = 0.445$, $u = 0.698$, $p = 3.528$ for $x = [0, 0.5]$, and
$\rho = 0.5$, $u = 0$, $p = 0.571$ for $x = (0.5, 0.1]$.
The computed instantaneous density and pressure profiles at $t = 0.15$
are compared with the analytical solution data, and plotted in the following figures.

!media docs/media/navier_stokes/rdg/fig_lax_dens.png width=50% padding-right=20px float=right caption=Lax-Harten shock tube: density profiles at $t = 0.15$.

!media docs/media/navier_stokes/rdg/fig_lax_pres.png width=50% padding-right=20px float=right caption=Lax-Harten shock tube: pressure profiles at $t = 0.15$.


###Input File

The content of some input file blocks is described in detail for clarity.

####GlobalParams

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=GlobalParams

Notes:

  1. It is mandatory to declare `order = CONSTANT` and `family = MONOMIAL`, which specifies the piecewise cell-average solution variable for the cell-centered finite volume method.
  2. It is convenient to provide some parameters for rDG-related objects, such as `slope_reconstruction = rslope` and `slope_limiting = lslope` for *slope reconstruction* and *slope limiting*.
  3. If an explicit time integration method is used, it is convenient to declare `implicit = false` here, so that Jacobian matrices will not be computed.

####Mesh

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=Mesh

Notes:

  1. A 1D domain ranging between $x=0$ and $x=1$ is defined, with 400 elements equally distributed in the domain.

####Functions

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=Functions

Notes:

  1. In this case, a piecewise constant function is used to specify the initial conditions in terms of $\rho$, $\rho u$, and $\rho e$.

####UserObjects

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=UserObjects

Notes:

  1. The prefix `CNSFV` represents "Compressible Navier-Stokes Finite Volume" --- an abbreviation to name these classes.
  2. `CNSFVSlopeReconstructionOneD` does not do any work, but has to be in place for code consistency. In 1D the slope reconstruction and limiting can be accomplished in one user object.
  3. `CNSFVSlopeLimitingOneD` calculates the limited slope for each element in 1D.
  4. `CNSFVHLLCInternalSideFlux` calculates the internal side flux using the HLLC approximate Riemann solver.
  5. `CNSFVFreeOutflowBoundaryFlux` calculates the boundary side flux using a free outflow BC.

####Variables

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=Variables

Notes:

  1. Declare the nonlinear variables for $\rho$, $\rho u$, and $\rho e$. The type and family of these variables are declared in `GlobalParams` for convenience.

####Kernels

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=Kernels

Notes:

  1. Always set `implicit = true` for time derivative kernels when using explicit time integration.
  2. In FVMs, there is no volumetric integration for flux terms.
  3. Other possible kernels in this block include source terms. In this example, we do not have any.

####DGKernels

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=DGKernels

Notes:

  1. Internal side flux terms should be declared in this block.

####BCs

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=BCs

Notes:

  1. Boundary side flux terms should be declared in this block.

####Materials

!listing modules/navier_stokes/tests/cnsfv/1d_lax_shock_tube.i block=Materials

Notes:

  1. `CNSFVMaterial` does not calculate actual material properties. It serves as an intermediate stage to host calls of slope reconstruction and limiting in every element, and then interpolate variable values at side centers.

##Examples in 2D

The code has passed four benchmark test problems in 2D:

1. inviscid flow through a channel,
2. supersonic flow over a wedge,
3. the cylindrical Sedov intense point explosion, and
4. a bow shock upstream of a blunt body in supersonic flow.

The problems listed above can be found in the navier_stokes test directory. The second problem is displayed here as an example.


###Supersonic Flow Over a Wedge


!media docs/media/navier_stokes/rdg/2d_obliqueshock_mesh.png width=45% padding-left=20px float=right caption=An unstructured mesh with 11,024 triangular cells and 5,656 points.

This is a standard problem in compressible, invicid shock theory, with details to be found in \cite{anderson1990modern}.
In this test case, a relatively coarse unstructured mesh is used for simulations;
see the figure on the right.
The cells are isotropic, and the mesh is relatively coarse for this problem.
Only half of the configuration is modeled due to symmetry of this problem.
The wedge has a half-angle of $15^{\circ}$, and a free-stream Mach-3 flow (with $\rho_{\infty}=1$ and $p_{\infty}=1/1.4$)
is imposed on the left inflow boundary.
A slip BC is imposed on the bottom symmetry boundary and wall surface of the wedge.
The Riemann invariant BC is imposed on the top and right outflow boundaries.
FV(1) was used in simulations with the minmax and HWENO limiters, respectively.
An attached oblique shock should form with a shock angle of about $32.2^{\circ}$.
The density and pressure behind the shock are given by
$\rho_{\tt post}/\rho_{\infty}=2.03$ and $p_{\tt post}/p_{\infty}=2.82$.
The Mach number just behind the shock should be $2.25$.


!media docs/media/navier_stokes/rdg/2d_obliqueshock_expl_hist.png width=45% padding-left=20px float=right caption=Computed density residual norms versus time steps.

The explicit TVDRK2 time-stepping scheme was used for time integration in this test case.
As shown in the figure on the right, convergence was monitored with regard to density residual norms.
Full convergence was achieved by FV(1) with the HWENO limiter, as the density residual norm decreased to the scale of $10^{-12}$.
In comparison, the density residual norm by FV(1) with the minmax limiter was stalled and oscillating at the scale of $10^{-4}$,
indicating the difficulty for FV(1) to reach convergence with the minmax limiter in this test case.


The defect associated to the minmax limited FV(1) solution is also exposed by the oscillations in the density and pressure contours computed at the end of simulations, as shown in the figures below. Although not significant, those oscillations in the post-shock region can be observed near the leading edge of the wedge. In comparison, FV(1) with the HWENO limiter resulted in fully converged smooth solutions in the post-shock region. Therefore, to solve problems containing strong shockwaves or discontinuities, we recommend the HWENO limiter over the minmax limiter.

!media docs/media/navier_stokes/rdg/2d_obliqueshock_expl_minmax_dens.png width=50% padding-right=20px float=left caption=Density contours by FV(1) with minmax limiter.

!media docs/media/navier_stokes/rdg/2d_obliqueshock_expl_weno_dens.png   width=50% padding-right=20px float=left caption=Density contours by FV(1) with HWENO limiter.


!media docs/media/navier_stokes/rdg/2d_obliqueshock_expl_minmax_pres.png width=50% padding-right=20px float=left caption=Pressure contours by FV(1) with minmax limiter.

!media docs/media/navier_stokes/rdg/2d_obliqueshock_expl_weno_pres.png   width=50% padding-right=20px float=left caption=Pressure contours by FV(1) with HWENO limiter.

Lastly, accuracy of the numerical schemes were examined through comparison between the numerical solutions and the analytical data in the figure below, where the computed density and pressure values are plotted along the horizontal line of $y = 0.35$. The minmax-based and HWENO-based FV(1) solutions agree well, and they have both resolved the shock with adequate accuracy, considering the coarseness of the mesh used in the simulations. Nevertheless, the HWENO limiter is more advantageous than the minmax limiter in this test problem, as the former is able to deliver oscillation-free solution over the whole post-shock region.

!media docs/media/navier_stokes/rdg/2d_obliqueshock_dens_profile.png width=50% padding-right=20px float=left caption=Density profiles along the horizontal line of $y = 0.35$.

!media docs/media/navier_stokes/rdg/2d_obliqueshock_pres_profile.png width=50% padding-right=20px float=left caption=Pressure profiles along the horizontal line of $y = 0.35$.

###Input File

The content of some input file blocks is described in detail for clarity.

####GlobalParams

!listing modules/navier_stokes/tests/cnsfv/2d_obliqueshock_expl_weno.i block=GlobalParams

Notes:

  1. It is mandatory to declare `order = CONSTANT` and `family = MONOMIAL`, which specifies the piecewise cell-average solution variable for the cell-centered finite volume method.
  2. It is convenient to provide some parameters for rDG-related objects, such as `slope_reconstruction = rslope` and `slope_limiting = lslope` for *slope reconstruction* and *slope limiting*.
  3. If an explicit time integration method is used, it is convenient to declare `implicit = false` here, so that Jacobian matrices will not be computed.

####Mesh

!listing modules/navier_stokes/tests/cnsfv/2d_obliqueshock_expl_weno.i block=Mesh

Notes:

  1. In this case, an ExodusII-format mesh is used.

####UserObjects

!listing modules/navier_stokes/tests/cnsfv/2d_obliqueshock_expl_weno.i block=UserObjects

Notes:

  1. The prefix `CNSFV` represents "Compressible Navier-Stokes Finite Volume" --- an abbreviation to name these classes.
  1. `CNSFVSlipBCUserObject` calculates the variable values on the ghost cells belonging to a slip boundary.
  1. `CNSFVFreeInflowBCUserObject` calculates the variable values on the ghost cells adjacent to a inflow boundary.
  1. `CNSFVRiemannInvariantBCUserObject` calculates the variable values on the ghost cells adjacent to a Riemann-invariant boundary; here it is actually imposed on the outflow boundary.
  1. `CNSFVLeastSquaresSlopeReconstruction` performs piecewise linear gradient reconstruction for each element using the least-squares method.
  1. `CNSFVWENOSlopeLimiting` performs the `HWENO` reconstruction for each element.
  1. `CNSFVHLLCInternalSideFlux` calculates the internal side flux using the HLLC approximate Riemann solver.
  1. `CNSFVHLLCSlipBoundaryFlux` calculates the slip boundary flux using a slip BC together with the HLLC approximate Riemann solver.
  1. `CNSFVFreeOutflowBoundaryFlux` calculates the boundary flux using a free inflow BC.
  1. `CNSFVRiemannInvariantBoundaryFlux` calculates the outflow boundary flux using a Riemann-invariant BC.

####Variables

!listing modules/navier_stokes/tests/cnsfv/2d_obliqueshock_expl_weno.i block=Variables

Notes:

  1. Declare the nonlinear variables for $\rho$, $\rho u$, $\rho v$, and $\rho e$. The type and family of these variables are declared in `GlobalParams` for convenience.

####Kernels

!listing modules/navier_stokes/tests/cnsfv/2d_obliqueshock_expl_weno.i block=Kernels

Notes:

  1. Always set `implicit = true` for time derivative kernels when using explicit time integration.
  2. In FVMs, there is no volumetric integration for flux terms.
  3. Other possible kernels in this block include source terms. In this example, we do not have any.

####DGKernels

!listing modules/navier_stokes/tests/cnsfv/2d_obliqueshock_expl_weno.i block=DGKernels

Notes:

  1. Internal side flux terms should be declared in this block.

####BCs

!listing modules/navier_stokes/tests/cnsfv/2d_obliqueshock_expl_weno.i block=BCs

Notes:

  1. Boundary side flux terms should be declared in this block.

####Materials

!listing modules/navier_stokes/tests/cnsfv/2d_obliqueshock_expl_weno.i block=Materials

Notes:

  1. `CNSFVMaterial` does not calculate actual material properties. It serves as an intermediate stage to host calls of slope reconstruction and limiting in every element, and then interpolate variable values at side centers.


##Reference

\bibliographystyle{unsrt}
\bibliography{docs/bib/rdg.bib}
