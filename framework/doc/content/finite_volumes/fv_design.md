
# Finite Volume Design Decisions in MOOSE

MOOSE has traditionally been a finite element (FE) framework.  It is built on
and takes heavy advantage of the libMesh FE library.  Traditionally, the
finite volume (FV) method doesn't really have shape-functions to describe
continuous solutions within mesh cells.  Instead it uses a constant solution
within each mesh element/cell. Because of this, much of MOOSE's use of libMesh
for FE is not relevant for FV.  This document gives an overview of how FV is
similar and different from FE with respect to implementation in MOOSE and
explains why FV is designed and implemented in its current form.  In order to
fully enable taking advantage of perfomance opportunities and to simplify the
FV method implementation in MOOSE, a new set of FV-specific systems was built
along-side the MOOSE FE infrastructure.  As a new set of systems being created
after MOOSE has received powerful automatic differentiation (AD) support, the
FV systems (at least initially) are only being created with AD support in
mind, and non-AD (manual Jacobian) versions will only be supported if a
pressing need arises.

## Variables

FV-specific (dependent) variable classes were create along side the
FE-specific ones sharing common base functionality.  This is responsible for
calculating and providing FV cell/face solution values to objects that need
them.  Higher-order reconstruction will also be plumbed into here eventually.
Ghost cells for boundary conditions and other important functionality is
handled automatically at this level so kernel and boundary condition code can
be written nearly like the FE equivalents.

Previously, the variable class hierarchy did not have a
field-variable-specific intermediate class.  FV variables, however are field
variables that need to be included in some field-variable operations performed
in MOOSE.  This was not previously possible because FV variables would have to
have shared their entire interface/API with FE variables - which would be a
poor choice because of so many non-overlapping API needs between the two (e.g.
no shape and test function related functions are needed for FV variables).  So
a new field-variable intermediate class was introduced to the variable class
heirarchy to facilitate appropriately separate APIs while allowing common
operations to be performed on all field variables.

## FV Kernels

Flux Kernels:

The FV method uses the Gauss-divergence theorem to convert
volume integrals with a divergence operator into surface integrals
representing flux of various quantities through faces between mesh cells.
Unlike FE kernels, no test/weight function is needed.  Coupling between cells
occurs from this numerical flux calculation on a face contributing to the mesh
cells on both sides of it (with opposing directional sign).  Calculating these
numerical fluxes requires access to variable values and properties on both
sides of each face.  FE kernels, on the other hand, require only the one set
of volumetric/elemental values for the cell of interest.  FV kernels also need
to deal with things such as normal face vectors, cross-diffusion correction
factors for non-orthogonal meshes, etc.  All these differences make it
impractical and messy to try to integrate them both into the same MOOSE kernel
system and motivated the decision to create a separate FV kernel system.

Elemental Kernels:

The FV method does have some element-based calculations
from source and time terms that are just handled/called through the normal FE
elemental residual and jacobian mesh loops.  Time derivative kernels and
source terms fall in this category.

## Shape Functions and Integration

Because some basic aspects of FV are "simpler" than FE, there is opportunity
to operate with a lower computational cost per element than with the FE
method.  The FV implementation does the following differently than FE:

* skips initialization/storage of shape-function data structures

* skips calculation of dependent variable values at quadrature-points - the
  coefficients (degrees of freedom) from the solution vector(s) can be
  directly used. FV only needs that one value per cell.

* skips element integration routines/loops not needed for FV's
  cell-constant solution.

Taking full advantage of these and other opportunities will take significant
work and FV performance can be expected to improve over time.  Some quadrature
point concepts have been retained for FV in MOOSE. This allows for future
expansion to higher-order cell solution variable representations in addition
to preserving similarity of APIs for users already familiar with MOOSE's other
objects and systems.

## Looping over Faces

Significant portions of the FV method are naturally cell-face oriented.
libMesh does not provide facilities for looping over and working with faces.
FV-specific data structures were created to facilitate looping over mesh faces
to compute residual contributions from numerical fluxes.

A face loop was implemented along side existing element, node, etc. loops used
for FE.  This is used for calculating numerical flux contributions from FV
kernels and boundary conditions.  The face info metadata needed for FV is
gathered up front (and recomputed whenever the mesh changes) and cached in
MOOSE's mesh data structure.  Needs with respect to Dirichlet boundary
conditions among other things influenced the decision to have a face info data
structure become a hub for objects to retrieve relevant information for
calculating residual contributions - more discussion about this is done in the
Boundary Conditions section.

## Boundary Conditions

Similar reasoning to decisions about the FV kernel system motivated the
creation of a separate FV boundary condition system as well.  While FV
flux/integrated boundary conditions are somewhat similar to FE integrated BCs,
they still lack test/weight functions.  FV Dirichlet BCs, however must be
implemented completedly differently than in FE and strongly motivate the
creation of a separate FV BC system.

Dirichlet boundary conditions (BCs) in an FV method cannot be created by
directly setting degrees of freedom like in an FE method because the FV
degrees of freedom do not exist on the mesh boundary.  There are various
approaches for dealing with this in FV.  A ghost-element approach was selected
due to its popularity and robustness.  In this approach, Dirichlet BCs are
implemented as a weak BC.  To do this, the normal flux kernel terms are
applied at the mesh boundary faces.  Since flux kernels are calculated using
information from cells on both sides of the face, we use the desired Dirichlet
BC value to extrapolate a "ghost" cell value for the side of the face that has
no actuall mesh cell.  Other necessary cell properties are also
reflected/mirrored from the existing cell.  A design was chosen that allows
handling ghost-element creation and use by existing flux kernels automatically
for enforcement of Dirichlet BCs.  This procedure results in the Dirichlet BC
objects not actually being responsible for calculating residual contributions.
They instead inform the ghost-element initialization while the normal flux BCs
are used to calculate boundary residual contributions.  This and other
differences motivated the creation of a separate FV BC system.

Many objects in MOOSE get information about the current mesh location by
directly accessing their own and inherited member variables.  This becomes
somewhat tricky to handle for FV because of the nature of ghost elements.
Traditionally, if an object needed the cell volume, it would access an element
pointer and use libMesh APIs.  This doesn't work for elements that don't
exist, yet we still need to provide this information for calculating Dirichlet
BC residual contributions.  We need to be able to provide information that
doesn't exist and we need to make sure code doesn't try to access irrelevant
or wrong information directly from the assembly, FEProblem, or other classes.
For this reason, among others, for FV objects the convention has been
established for objects to retrieve needed information from a face info object
that is passed around rather than retrieving binding references to the usual
mesh-related data.  If everyone gets needed information from this one place -
it is easy to monitor when code may be doing the wrong thing.  It also becomes
a simple matter to provide volumes for non-existing cells and add features
that require intercepting and modifying any face information objects need.

## Reconstruction

Gradient reconstruction is implemented using the Green-Gauss method, e.g.

\begin{equation}
\nabla \phi_C = \frac{1}{\Omega} \sum\limits_{faces} \phi_f \vec{S}_f
\end{equation}

where $\phi_f$ is the value of the quantity of interest on the face and
$\vec{S}_f$ is equal to the surface area times the outward normal,
e.g. $\vec{S}_f = A\hat{n}$. The value of $\phi_f$ can be computed using central differences
with or without skewness-correction.
The skewness-correction of the compact (central-difference) stencil can be expressed as:

\begin{equation}
\phi_f = \phi_{f'} + \nabla \phi_{f'} (\vec{r}_{f'}-\vec{r}_f),
\end{equation}

meaning that the approximate face value ($\phi_{f'}$) at the intersection of
the line connecting the cell centroids and the face ($\vec{r}_{f'}$) is corrected using the
approximate gradient at that point and a correction vector $(\vec{r}_{f'}-\vec{r}_f)$.
This yields second order convergence on [skewed](skewness-correction/adv-diff-react/skewed.i)
meshes where the compact stencil falls back to first order. However, this
comes with an additional computational cost in the system assembly process.

Due to its reduced stencil size (smaller memory footprint for
the Jacobian matrix), its reduced computational expense, and demonstrated
convergence properties, we generally recommend use of the compact stencil. The
skewness-corrected stencil is demonstrated to be more accurate, however it
considerably slows (a factor of approx. 2-3 depending on the caching options)
down the assembly process due to the fact that additional face gradients need to be computed.

On regular, orthogonal grids, the face gradient $\nabla \phi_f$ can be computed
using a simple linear interpolation between neighboring cell gradients,
e.g. between $\nabla \phi_C$ and $\nabla \phi_F$. However, on non-orthgonal
grids, some correction has to be made. The correction implemented is that shown
in section 9.4 of [!cite](moukalled2016finite):

\begin{equation}
\nabla \phi_f = \overline{\nabla \phi}_f +
    \left[\frac{\phi_F - \phi_C}{d_{CF}} - \left(\overline{\nabla \phi}_f \cdot \vec{e}_{CF}\right)\right]
    \vec{e}_{CF}
\end{equation}

where $\overline{\nabla \phi}_f$ denotes the linearly interpolated face pressure
gradient.

## Known Limitations/Issues

* FE <---> FV variable-coupling does not work.  Particularly, if FE
  variables try to couple to an FV variable, they will segfault when trying to
  access quadrature points at any index higher than zero.  Also, FV variables
  coupling to FE variables should ideally get a cell-averaged value - but
  currently, they will just get the value of the solution at the cell's
  0-index quadrature point. See
  [idaholab/moose#15062](https://github.com/idaholab/moose/issues/15062)

* FV functionality does NOT work with mesh displacements yet. See
  [idaholab/moose#15064](https://github.com/idaholab/moose/issues/15064)

* Have not tested vector-FV varaibles - they almost certainly don't work (yet).
