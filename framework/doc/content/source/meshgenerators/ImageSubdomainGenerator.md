# ImageSubdomainGenerator

!syntax description /Mesh/ImageSubdomainGenerator

## Example Syntax

Normal usage of ImageSubdomainGenerator involves creation of a standard mesh (in the example, `GeneratedMeshGenerator`), using that mesh as the input for ImageSubdomainGenerator, and providing an image to sample. See an example below:

!listing test/tests/preconditioners/fsp/fsp_test_image.i block=Mesh

!alert note
In this example, the parameter `threshold` is used. This sets a color value above which the subdomain ID is set to `upper_value` (default = 1) and below which the subdomain ID is set to `lower_value` (default = 0). More information about these and other extended parameters can be seen below in the Input Parameters section.

### Input Image (kitten.png)

!media media/framework/mesh_modifiers/kitten.png style=width:250px;float:center;margin-left:40px

### Mesh Subdomain ID Output

!media media/framework/mesh_modifiers/kitten_out.png style=width:550px;float:center;margin-left:40px

!syntax parameters /Mesh/ImageSubdomainGenerator

!syntax inputs /Mesh/ImageSubdomainGenerator

!syntax children /Mesh/ImageSubdomainGenerator
