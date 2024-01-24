# Application System

## Description

The `Application` block within an input file is utilized to explicitly specify the application type used for the input file. The application type should be provided using the [!param](/Application/type) parameter in the `Application` block. This block is parsed before the MOOSE application is actually built. If any mismatch between the registered application type and the user-selected type is detected, the system will immediately throw an error and stop the setup of the simulation.

## Example

The following input file snippet demonstrates the use of the `Application` block.

!listing test/tests/multiapps/application_block_multiapps/application_block_sub.i block=Application


!syntax list /Application objects=True actions=False subsystems=False

!syntax list /Application objects=False actions=False subsystems=True

!syntax list /Application objects=False actions=True subsystems=False
