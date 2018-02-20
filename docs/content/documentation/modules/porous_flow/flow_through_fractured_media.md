# Flow through an explicitly fractured medium and flow through a fractured network

Philipp Sch&#228;dle, Andy Wilkins, Anozie Ebigbo and Martin O Saar  

Example files are in [flow_through_fractured_media](https://github.com/idaholab/moose/blob/master/modules/porous_flow/examples/flow_through_fractured_media).

## Introduction

The PorousFlow module may be used to simulate flow (both fluid and
heat) through a medium that contains explicit fractures.  This may be
performed in two ways:

1. A simulation may use a 3D mesh that contains thin 3D elements to
 represent the fractures.  Usually the thin "fracture" elements are
 part of a separate block in the mesh to which appropriate porosity
 and permeability are prescribed.  No special input-file magic needs
 to be performed.  The same remarks hold for a 2D mesh containing thin
 2D elements representing the fractures.

2. A simulation may use a **3D** mesh that contains **2D**
 elements to represent the fractures.  Of course this is slightly
 more numerically efficient than the aforementioned method, but,
 most importantly, the mesh is usually easier to create.  This
 example concentrates on this method.  Similar remarks hold for a 2D
 mesh containing 1D elements.  *This method is only valid if flow
 across the fracture is very quick compared with flow through the
 bulk.  For instance, the method is valid for highly conductive
 fractures, but is **not** suitable for modelling aquitards.*

An overview of the mixed-dimensional method is presented along with a
verification of the results for solute transport along a single
fracture embedded in a porous matrix.

An identical method may be used to simulate flow through a fractured
network (ie, 2D elements without the "bulk" 3D elements).

## The mesh

The key aspect of the formulation presented here is the mesh.  It is a
mixed-dimensional, conforming mesh. That is, the mesh needs to be
generated such that the lower-dimensional fracture elements share
nodes with the porous-matrix elements.  On these nodes, computational
variables take on a single value, and the nodes physically connect the
subdomains.  In real fractured materials a good mesh may be very
difficult to create because of the complicated intersection of many
different fractures.

\ref{MeshDetailFig} shows an example of the
required configuration for the conforming mesh and the different
sub-domains.

!media media/porous_flow/mixed-dimensional-mesh.png width=80%
margin-left=10px caption=(a): The mesh used by MOOSE, which is a mixed-dimensional conforming mesh containing 1D and 2D elements.  (b): An exploded view.  Nodes $n_1$, $n_2$, $n_3$ need to be shared by the three domains.  id=MeshDetailFig

When creating a mesh:

 - For the case of 2D elements embedded in a 3D porous matrix, the 2D
 elements need to be QUAD or TRI elements that are connected to the 3D
 elements.
 
 - For the case of 1D elements embedded in a 2D porous matrix, the 1D
 elements need to be BAR elements that are connected to 2D QUAD or TRI
 elements.
 
In the case of an isolated fracture network, the mesh needs to consist
of QUAD or TRI elements (for 2D fractures) or BAR elements (for 1D fractures).

Finally, the fractures elements _need to be part of a separate
subdomain (block)_.  This is actually a requirement of the exodus
format, but most importantly it allows permeability, porosity, heat
conductivity, etc, to be correctly prescribed.

In this example, the model consists of a 2D
domain with a single, 1D fracture embedded in a porous matrix.

## Material properties

The second aspect of the mixed-dimensional method is prescribing
appropriate material properties (permeability, porosity, conductivity,
etc) to the fracture blocks.  Although the details below may seem
daunting the conclusion is simple: *just multiply the experimentally
measured fractured permeability, porosity, etc by the fracture
aperture and enter this number in your input file*.

### Residuals

Consider the MOOSE residual for a single 3D element corresponding to
the time derivative of the fluid mass.  It is
\begin{equation}
R_{3} = \int_{V} \frac{\partial}{\partial t} \phi\rho S  \ .
\end{equation}
In this equation $R_{3}$ is the residual, with the "3" subscript
indicating that this is a residual coming from a 3D element, and $V$
is the volume of the element.  The other notation is detailed in
[nomenclature](/porous_flow/nomenclature.md).

Similarly, consider the MOOSE residual for a single 2D element
corresponding to the time derivative of the fluid mass.  It is
\begin{equation}
R_{2} = \int_{A} \frac{\partial}{\partial t} \tilde{\phi}\rho S \ .
\end{equation}
Here $\tilde{\phi}$ has been used to emphasise that this "2D" porosity
is not necessarily the same as the 3D porosity, $\phi$.

When constructing the overall nonlinear residual, MOOSE sums all the
residuals from the individual elements (ie, MOOSE sums all the
$R_{3}$'s and all the $R_{2}$'s).  The "2D" porosity $\tilde{\phi}$
must be chosen correctly so that the contributions from $R_{2}$ are
weighted appropriately.

To do this, think of the 2D elements as very thin 3D
elements. Assuming there is no dependency on the third direction, as
is appropriate because the 3D element is very thin, the residual for
these thin elements is
$$
R_{3} = a \int_{A} \frac{\partial}{\partial t} \phi\rho S \ ,
$$
where $a$ is the thickness in the third direction.  We want this to be
identical to $R_{2}$, which immediately implies
$$
\tilde{\phi} = a\phi \ .
$$

Similar arguments may be made for the fluid flow or heat flow Kernels of PorousFlow (and indeed, the entire PDEs) and these result in equations such as
$$
\tilde{k}_{ij} = a k_{ij} \ ,
$$
where $k$ is the permeability tensor.

The tildered parameter values are those that should be used in the
MOOSE input file, for the Material properties of the blocks associated
to the 2D elements.

### Example

For instance, suppose that a sample of the fracture is experimentally
measured to have porosity $\phi=0.5$.  Perhaps this fracture is a
sand-filled planar inclusion in a granite rock mass, for instance.
Obviously this is a 3D result since it exists in real life.  Suppose
the fracture is known to have thickness 0.1m.  Then when this fracture
is represented as a 2D element in MOOSE it should be given porosity
$\tilde{\phi} = 0.1\times 0.5 = 0.05$ (with units of metres).

### Relationship to the $a^2$ fracture formula - the cubic law

It is known that fluid flow between a pair of impermeable parallel
plates may be well-approximated by Darcy flow through an effective
medium with permeability $k = a^2/12$, where $a$ is the separation of
the plates.  In this setup the fluid flow is unimpeded by obstructions
or similar things between the parallel plates.  The parallel plates
are not included in this effective medium: it is simply a slab of
material with permeability $a^2/12$.  This effective medium may be
represented by 3D elements in MOOSE and simulated in the usual way, by
setting the permeability in the input file to $a^2/12$.

How is this $a^2/12$ related to the formulae presented above?  Well,
the user may set $k=a^2/12$, which would be appropriate if their
fracture was well approximated by parallel plates, and then, assuming
the fratures are modelled by 2D elements in a 3D mesh, use the
formula $\tilde{k}=ak$ to prescribe the permeability to the 2D
fractures (probably along with $\phi=1$ which implies
$\tilde{\phi}=a$).  But users don't strictly _need_ to do this: any
$k$ that is deemed appropriate may be used, such as a value derived from
experimental data measured in real (3D) experiments performed on the
material the fracture is made from.  Such a value is unlikely to
depend on $a$, because it will be dictated by the material present
within the fracture (eg, sand) which is assumed not to be present when
deriving the $a^2/12$ formula.

## Verification


### Meshes used

For the verification of this approach, two different simulations are
run on two different meshes.  The bulk porous material is 2D.

1. In the first simulation, the fracture is geometrically represented
by 2D elements with a thickness that corresponds to the aperture of
the fracture.  This is the reference case with which the
lower-dimensional fracture case is to be compared.

2. In the second simulation, the fracture is modeled by
lower-dimensional (1D) elements.

The discretization along the fracture is similar in both
cases. Further, the discretization in the porous matrix perpendicular
to the fracture increases logarithmically and is similar for both
models. \ref{MeshFig} shows a close-up of the region near the fracture
for the two different meshes.  The results of these two simulations
are compared for verification.

!media media/porous_flow/fracture_zoom_fine.png width=100% margin-left=10px caption=Close-up of the area near the fracture (red) in a porous matrix (blue) as represented by the two meshes. In (a), the fracture is represented by 2D elements whose heights correspond to the aperture of the fracture.  In (b), the fracture is represented by 1D elements that are embedded in the "bulk" 2D elements.  id=MeshFig

### Problem description

Single-phase fluid flow and transport of one solute though a fracture
and adjacent porous matrix is simulated.  The porous matrix is assumed
to have a low permeability, so that transport in the matrix is
mainly diffusive.

Steady-state flow equations are solved to obtain the pressure
field. This is followed by a transient simulation of the transport of
the solute.  Dirichlet boundary conditions are applied for pressure
and solute mass fraction at the left and right ends of the
fracture. The pressure difference of 0.002$\,$MPa results in laminar
fluid flow from right to left.  Similarly, the mass fraction at the
right boundary is $1$ and at the left boundary $0$.  The dimensions of
the model are 1$\,$m along the fracture and 0.2$\,$m
perpendicular to the fracture.

For the 1D fracture case, it is assumed that the fracture aperture is
constant within each fracture element. Hence, in order to account for
the missing dimension, the 1D mass balance equation is multiplied by
the aperture. This is realized in the MOOSE input file by multiplying
the parameters for fracture porosity and fracture permeability by the
aperture as described [Material properties](#material-properties).

 The
permeability of the fracture is calculated as a function of aperture,
$a$, using the cubic law:
\begin{equation}
k_\text{f}=\frac{a^2}{12} \ .
\end{equation}
The remaining relevant input parameters are tabulated in \ref{ParameterTable}

!table id=ParameterTable caption=Input parameters
| Parameter | Case with 2D fractures | Case with 1D fractures |
| - | - | - |
| Aperture | $a = 6\times 10^{-4}\,$m | $a = 6\times 10^{-4}\,$m |
| Permeability, fracture | $k_{\text{f}} = 3\times 10^{-8}\,$m$^{2}$ | $\tilde{k}_{\text{f}} = ak_{\text{f}} = 1.8\times 10^{-11}\,$m$^{3}$ |
| Permeability, matrix | $k_{\text{m}} = 10^{-20}\,$m$^{2}$ | $k_{\text{m}} = 10^{-20}\,$m$^{2}$ |
| Porosity, fracture | $\phi_{\text{f}} = 1$ | $\tilde{\phi}_{\text{f}} = a\phi_{\text{f}} = 6\times 10^{-4}\,$m |
| Porosity, matrix | $\phi_{\text{m}} = 0.1$ | $\phi_{\text{m}} = 0.1$ |
| Tortuosity, fracture |   1 |    1 |
| Tortuosity, matrix |     0.1 |  0.1 |
| Diffusion coefficient | $10^{-9}\,$m$^{2}$.s$^{-1}$ | $10^{-9}\,$m$^{2}$.s$^{-1}$ |

The important parts of the mixed-dimensional input file are

!listing /modules/porous_flow/examples/flow_through_fractured_media/fine_transient.i start=[./permeability_fracture] end=[]

!listing /modules/porous_flow/examples/flow_through_fractured_media/fine_transient.i start=[./poro_fracture] end=[./poro_fracture_nodal]


### AuxKernels

Given the alterations to the input parameters to account for the
lower-dimensional domain, it is important to take care when
calculating velocities using PorousFlow Auxkernels. The
[PorousFlowDarcyVelocityComponentLowerDimensional](/systems/AuxKernels/porous_flow/PorousFlowDarcyVelocityComponentLowerDimensional.md)
AuxKernel uses the correct gravitational vectors in the
lower-dimensional domain. However, the velocities calculated by this
AuxKernel only have units of m.s$^{-1}$ if the aperture is provided to
the AuxKernel.  The aperture does not need to be supplied to the bulk
Darcy velocities computed by
[PorousFlowDarcyVelocityComponent](/systems/AuxKernels/porous_flow/PorousFlowDarcyVelocityComponent.md).

Also note that these are not
the velocities seen by an observer watching a tracer move through the
medium.  To obtain those velocities, the bulk (3D) Darcy velocity must
be divided by the porosity.  The fracture Darcy velocity must be
divided by $\phi_{\text{f}}$, or $\tilde{\phi}_{\text{f}}$ if the
aperture is not provided to [PorousFlowDarcyVelocityComponentLowerDimensional](/systems/AuxKernels/porous_flow/PorousFlowDarcyVelocityComponentLowerDimensional.md).

Finally, it is important to note that the velocity AuxVariables
defined for the fracture domain should be `family=MONOMIAL` and
`order=CONSTANT` in order to display accurate velocities using
Paraview.

The following mixed-dimensional input-file blocks define Darcy velocity on a
lower-dimensional fracture, with the result having units of m.s$^{-1}$.

!listing /modules/porous_flow/examples/flow_through_fractured_media/fine_transient.i start=[AuxVariables] end=[ICs]



### Results

\ref{result1.fig} and \ref{result2.fig} show results of the solute-transport problem.  The advection of the solute is shown as well as its penetration into the porous matrix.  The results are identical for the two different meshes, demonstrating the correctness of the mixed-dimensional approach.


!media media/porous_flow/fracture_flow_result_sum_contour_0_0_0.png width=100% margin-left=10px caption=Mass fraction along the fracture for each of the first 20 seconds of simulation.  id=result1.fig

!media media/porous_flow/fracture_flow_result_sum_contour_1_0_4.png width=100% margin-left=10px caption=Mass fraction perpendicular to the fracture at location 0.1$\,$m from the injection point after 24 hours of simulation.  id=result2.fig


## A 3D example

Identical methods may be used to simulate flow in 3D.  An example mesh
and an input file may be found in
[flow_through_fractured_media](https://github.com/idaholab/moose/blob/master/modules/porous_flow/examples/flow_through_fractured_media).  Two intersecting eliptical fractures are embedded in a 3D porous material.  A porepressure gradient is established, and a tracer is injected at the edge of one of the fractures.  The tracer flows mainly along the fractures, but diffuses a little into the bulk material.  \ref{3D_pic} shows the result at one time, and \ref{3D_animation} shows an animation of the tracer concentration.

!media media/porous_flow/fracture_flow_3D.png width=80% margin-left=10px caption=Tracer mass fraction (red=high) in two intersecting 2D fractures contained in a 3D porous medium.  id=3D_pic

!media /porous_flow/fracture_flow_3D.gif width=80% margin-left=10px caption=Tracer mass fraction in the 2D fractures as time progresses.  id=3D_animation


