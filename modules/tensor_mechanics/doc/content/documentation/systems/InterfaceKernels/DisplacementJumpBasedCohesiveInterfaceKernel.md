# Displacement Jump Based Cohesive Interface Kernel

!syntax description /InterfaceKernels/DisplacementJumpBasedCohesiveInterfaceKernel


This class implements a displacement jump based InterfaceKernel that can be used
for a general non-stateful cohesive law.
Because the residual of each displacement is teh traction, the weak form of the problem can be written as

\begin{equation}
\int_{\Gamma} T_i \cdot \left[ \Psi \right] d\Gamma
\end{equation}

where $\Gamma$ is the cohesive interface where this kernel operates, $T$ represents the traction on the master side of the interface, $\Psi$ represents the test function and $i$ is the index of the Traction on which this kernel is working on.
The symbol $\left[ \cdot \right]$ represent the jump of a variable between the interface and is defined as $\left[ x \right] = x^{slave} - x^{master}$

Because this kernel works only on one component of the Traction, one kernel for each mesh dimension need to be defined.
The parameter `disp_index` identify the component $i$ as:

- 0 $=>$ x

- 1 $=>$ y

- 2 $=>$ z

The parameter `var` and `nighbor_var` couple the displacement components on which this kernel is working on, i.e. the one identified by `disp_index`.

The parameters `disp_1` and `disp_2` (and the associated neighbors) couple the variables representing the other displacement components

- `disp_index=0`  $=>$  `disp_1=y` (in 3D `disp_2=z`)

- `disp_index=1`  $=>$  `disp_1=x` (in 3D `disp_2=z`)

- (in 3D `disp_index=2`  $=>$  `disp_1=x` `disp_2=y` )


To work this kernels requires to define a an interface material and an associate user object.
The interface material provides the coefficients of the residual and jacobian in the global coordinate system. The userobject implements the cohesive law in natural element coordinates


## Example Input File Syntax


### 2D

#### InterfaceKernel

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D.i block=InterfaceKernels


#### InterfaceMaterial

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D.i block=Materials/gap

#### UserObject for cohesive law

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D.i block=UserObjects/TractionSeparationTest


### 3D

For 3D simulations one needs to add an additional kernel

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/3D/czmTest3DC_CohesiveLaw3D.i block=InterfaceKernels



!syntax parameters /InterfaceKernels/DisplacementJumpBasedCohesiveInterfaceKernel

!syntax inputs /InterfaceKernels/DisplacementJumpBasedCohesiveInterfaceKernel
