# TorchScriptUserObject

!syntax description /UserObjects/TorchScriptUserObject

## Description

This user object loads a torch module that has been exported from
pytorch in a torch script format. The user object loads the modules
when the user object is execute which can be controlled using the
`execute_on` flag.

For more information on how to save a torch module using a
torch script format, visit the [official documentation](https://pytorch.org/docs/stable/jit.html).

## Example Input Syntax

!listing test/tests/materials/torchscript_material/test.i block=UserObjects

!syntax parameters /UserObjects/TorchScriptUserObject

!syntax inputs /UserObjects/TorchScriptUserObject

!syntax children /UserObjects/TorchScriptUserObject
