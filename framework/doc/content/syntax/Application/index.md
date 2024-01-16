# Application System

## Description

The `Application` block within an input file is utilized to explicitly specific the application type used for the input file. The application type should be provided using `type` keyword under `Application` block. This block is parsed even before the MOOSE application is actually built. If any mismatch between registered type and given type is detected, the system will immediately throw out an error and stop creating the rest MOOSE objects.

## Example

The following input file snippet demonstrates the use of `Application` block.

!listing test/tests/multiapps/application_block_multiapps/application_block_sub.i block=Application


!syntax list /Application objects=True actions=False subsystems=False

!syntax list /Application objects=False actions=False subsystems=True

!syntax list /Application objects=False actions=True subsystems=False
