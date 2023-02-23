# ElementIDOutputAction

!syntax description /Mesh/ElementIDOutputAction

This action is triggered when [!param](/Outputs/Exodus/output_extra_element_ids) is set to `true`. AuxVariables and AuxKernels relevant to the extra element ids defined on the mesh are automatically added to the problem, and the resulting element integers are also outputted to the Exodus file.

!listing test/tests/outputs/exodus/exodus_elem_id.i block=Outputs

More information can be found on the [Exodus output documentation page](outputs/Exodus.md).

!syntax parameters /Mesh/ElementIDOutputAction

!syntax inputs /Mesh/ElementIDOutputAction

!syntax children /Mesh/ElementIDOutputAction
