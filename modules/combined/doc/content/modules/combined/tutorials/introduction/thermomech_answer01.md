> Where is coupling introduced between the thermal and mechanical solutions in
> this model?

The only place where coupling between the temperature and displacement solutions
explicitly appears in the input file is in the block that defines the thermal 
eigenstrain:

!listing modules/combined/tutorials/introduction/thermal_mechanical/thermomech_step01.i block=Materials/expansion1

In this block, the `temperature` parameter indicates which variable is used to
define the temperature field. In this case, the `T` variable that is specified
is a solution variable. This results in coupling between the thermal and
mechanical solutions. Note that this specified variable can be either a variable
that is part of the solution, as is the case here, or an auxiliary variable,
which might be prescribed using a function, as was the case in the mechanics-only
[model](tensor_mechanics/tutorials/introduction/step03a.md) that this case builds
upon.

In addition to the dependency of the eigenstrain on the temperature solution,
there is also a dependency of the thermal solution on the mechanical solution
that is not as immediately apparent. The thermal kernels by default run on the
displaced mesh, the distortion of which is computed by the mechanics models.
For many models, this has a negligible effect, but it can be important in some
cases, particularly when thermal contact is included.
