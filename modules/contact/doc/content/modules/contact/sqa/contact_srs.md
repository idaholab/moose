!template load file=sqa/module_srs.md.template category=contact module=Contact

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Contact module provides capabilities for enforcing a variety of types of mechanical contact constraints between surfaces on deforming bodies. These constraints typically enforce the condition that the two surfaces do not penetrate each other. They can also optionally enforce that the two surfaces do not separate or slide relative to each other, or allow for slip only when the frictional capacity has been reached. Multiple methods are provided for enforcing these constraints, including node-on-face and mortar-based formulations. In addition to providing the capabilities for enforcing contact constraints, the Contact module also provides various supporting tools to facilitate setting up these models and outputting the results related to contact.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the [!ac](MOOSE) Contact module is to enforce mechanical contact constraints between opposing interacting surfaces of models of deforming bodies.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module has no constraints on hardware and software beyond those of the [MOOSE framework](framework_srs.md#assumptions-and-dependencies) and the [Tensor Mechanics](tensor_mechanics_srs.md#assumptions-and-dependencies) module.
The {{module}} module provides access to a number of code objects that perform computations. These objects each make their own physics-based assumptions, such as the units of the inputs and outputs. Those assumptions are described in the documentation for those individual objects.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 87% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!
