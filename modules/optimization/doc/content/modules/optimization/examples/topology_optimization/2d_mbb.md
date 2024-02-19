# 2D MBB Beam with a Convolution Filter

In this example we will go through the general setup of a topology optimization
problem using solid isotropic material penalization (SIMP), see [!cite](sigmund200199).
The problem is to find the optimal material distribution in a 2D
domain that minimizes the compliance of the structure. We will first define the
problem parameters. Below is a list of the parameters that we will use in this
example corresponding to the volume fraction, Young's modulus of the material,
and the penalization power. We will go over the material that is needed for the
setup and running the optimization problem, but skip over any information that
is covered in other MOOSE tutorials.

```
vol_frac = 0.5
E0 = 1
Emin = 1e-8
power = 3
```

Next we define the mesh and add the necessary nodesets.

!listing examples/optimization/2d_mbb.i
         block=Mesh id=mesh_block
         caption=MBB `Mesh` block

In the `AuxVariables` block there are two initial conditions.  The first is a
constant, negative value that is needed for the sensitivity variable `Dc`. It
needs to be negative for the first density update. The second initial condition
is setting the material density to the initial value of `vol_frac`.

!listing examples/optimization/2d_mbb.i
         block=AuxVariables id=aux_var_block
         caption=MBB `AuxVariables` block

The next block is the `Materials` block.  The first material is the "SIMP"
density altered young's modulus material. The material follows the form `E =
Emin + (density^penal) * (E0 - Emin)`. The second material is the compliance
sensitivity, which is used for updating the density field.

!listing examples/optimization/2d_mbb.i
         block=Materials id=mat_block
         caption=MBB `Materials` block

The final block is the `UserObjects` block.  This block contains the main
optimization functionality. First is the `RadialAverage` and `SensitivityFilter` objects that filter the
sensitivity to prevent checkerboarding (see [!cite](sigmund2007)).
The radius of the filter sets the
minimum size of a feature in the structure. Finally is the `DensityUpdate`
object that updates the density field based on the sensitivity and the keeps the
volume constraint satisfied.

!listing examples/optimization/2d_mbb.i
         block=UserObjects id=uo_block
         caption=MBB `UserObjects` block
