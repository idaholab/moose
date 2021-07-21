# GeneralUserObject

The GeneralUserObject intermediate base class is for creating custom algorithms that are not associated with
any mesh entity. By default, a GeneralUserObject will be executed after AuxiliaryKernels. If an AuxiliaryKernel depends on a GeneralUserObject, then the GeneralUserObject will be run before the AuxiliaryKernels for the execution flags on which the the AuxiliaryKernels executes. This default behavior can be changed through use of two parameters. The GeneralUserObject can be forced to execute before the AuxiliaryKernels regardless of any dependencies by setting the parameter `force_preaux` to true. If an initial condition depends on a GeneralUserObject, the GeneralUserObject is executed before the initial conditions. Execution of the GeneralUserObject can also be forced to occur before the initial conditions by setting the parameter `force_ic` to true. If `force_ic` is false and no initial conditions depend on the GeneralUserObject, it is executed after the AuxiliaryKernels.

<!-- use of /UserObjects/Terminator is arbitrary,
     just want to show the parameters force_preaux force_preic
     with the normal style but GeneralUserObject is not a registered
     moose object -->

!syntax parameters /UserObjects/Terminator show=force_preaux force_preic
