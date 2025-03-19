# TorchScriptMaterial

This object utilizes a torch script module to populate multiple material properties.
For the evaluation of the object, the inputs can be provided through postprocessor values
using the [!param](/Materials/TorchScriptMaterial/input_names) parameter. The
torch script module can be passed to this object using the
[!param](/Materials/TorchScriptMaterial/torch_script_userobject). For more
instructions on how to load the torch script material, we recommend visiting
[TorchScriptUserObject.md].

## Example Input Syntax

!listing test/tests/materials/torchscript_material/test.i block=Materials

!syntax parameters /Materials/TorchScriptMaterial

!syntax inputs /Materials/TorchScriptMaterial

!syntax children /Materials/TorchScriptMaterial
