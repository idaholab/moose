# ImageFunction

!syntax description /Functions/ImageFunction

## Overview

Simulations often require the phase to be initialized or compared to existing micro-structure data,
such as CT or SEM scans as shown in the figure. MOOSE includes a flexible,
[Function](Functions/index.md) based method for accessing this type of data for use in initial
conditions or other calculations. For example, [fig:large_adapt] is a reconstructed image of
snow captured using cryospheric micro-CT scanner at the
[MSU Subzero Research Laboratory](http://www.montana.edu/subzero).

!media image_function/large_adapt.png id=fig:large_adapt
       caption=Mesh of 3D micro-CT data using `ImageFunction`.

## Setup

To utilize the image reading portion of the phase-field module VTK must be enabled when building
libMesh. Please see MOOSE [FAQ](faq.md#libmesh-vtk optional=True) for more information.

## Examples

### Single Image

Consider the simple image in [fig:small_raw], reading this image is accomplished by
including a `ImageFunction` in your input file.

!listing image_function/image.i block=Functions

!media image_function/small_raw.png id=fig:small_raw
       style=width:50%;margin-right:auto;margin-left:auto;
       caption=Example image input (stack/test_00.png).


This function may then be used just like any other function within MOOSE, for example, it may be
utilized as an initial condition for a variable.  The following input file syntax would use the
function above as an initial condition for the `u` variable and create an initial mesh as shown in
[fig:small_uniform].

!listing image_function/image.i block=ICs

!media image_function/small_uniform.png id=fig:small_uniform
       style=width:50%;margin-right:auto;margin-left:auto;
       caption=Mesh showing initial condition extracted from image in [fig:small_raw].

The example image shown is 20 by 20 pixels as is the mesh (20 by 20 elements) to which the initial
condition is applied. The meshed version looks slightly different than the original image because
the initial condition is applied by sampling the image at each node within the mesh, which in this
case matches with the pixel boundaries, so the value can sampled can easily vary due to floating
point precision limitations.

Matching the mesh to the pixel dimensions +is not+ a requirement, and not recommend. The main reason
for building the `ImageFunction` object was to enable an arbitrary mesh geometry to be able to sample
the image and adapt accordingly.

Beginning with a 2 by 2 element mesh and adding the following an adaptivity block to the input file
results in the mesh shown [fig:small_adapt].

!listing image_function/threshold_adapt.i block=Adaptivity

!media image_function/small_adapt.png id=fig:small_adapt
       style=width:50%;margin-right:auto;margin-left:auto;
       caption=Mesh showing initial condition with initial adaptivity extracted from image in
               [fig:small_raw].

### Image Stacks

Image stacks or 3D images are also supported (see [#file-types]). For example, consider as set of
images named test_00.png, test_01.png, ..., and test_19.png within a directory "stacked". To read
these images the syntax below is used in the `ImageFunction` block.  Again, using this data as an
initial condition and using initial adaptivity, as shown above, results in the mesh shown in
[fig:small_3d].

!listing image_function/image_3d.i block=Functions

!media image_function/small_3d.png id=fig:small_3d
       style=width:50%;margin-right:auto;margin-left:auto;
       caption=3D Mesh showing initial condition with initial adaptivity extracted from a stack of
               images similar to the image in [fig:small_raw]. The mesh is cropped in the vertical
               direction to show the internal structure.

It is also possible to limit the reader to a set of images using the
[!param](/Functions/ImageFunction/file_range) parameter, which may be set to a single value to read a
single image or a range to read a subset of the images.


## Image Processing

The [VTK](http://www.vtk.org/) library includes a range of image filters and processing tools, some
basic processing tools are included. However, a derivative class could easily be developed to expand
upon these capabilities.

### Component

By default, the RGB pixel data is converted into a single greyscale value representing the
magnitude. This is accomplished using the
[vtkImageMagnitude](http://www.vtk.org/doc/nightly/html/classvtkImageMagnitude.html) class.

It is possible to select a single component rather than using the magnitude by setting the
[!param](/Functions/ImageFunction/component) parameter in the input file to a valid component number,
which will be 0, 1 or 2 for RGB images.

### Thresholding

Basic thresholding is accomplished using the
[vtkImageThreshold](http://www.vtk.org/doc/nightly/html/classvtkImageThreshold.html)
class. Thresholding requires three parameters be set in the input file:

- [!param](/Functions/ImageFunction/threshold): The threshold value to consider.
- [!param](/Functions/ImageFunction/upper_value): Image data above the threshold are replaced with
  this value.
- [!param](/Functions/ImageFunction/lower_value): Image data below the threshold are replaced with
  this value.

### Shift and Scale

It is possible to shift and scale the image data, this is accomplished using the
[vtkImageShiftScale](http://www.vtk.org/doc/nightly/html/classvtkImageShiftScale.html) object. The
[!param](/Functions/ImageFunction/shift) parameter adds the given value to the image data and
[!param](/Functions/ImageFunction/scale) parameter multiplies the image data by the supplied value.

The order of application of the shift and scale are dictated by the VTK object, the documentation
states: "Pixels are shifted (a constant value added) and then scaled (multiplied by a scalar)."

### Image Flipping

Flipping an image along the major axis directions x, y, or z is performed using
[vtkImageFlip](http://www.vtk.org/doc/nightly/html/classvtkImageFlip.html) object. Three flags
exists---[!param](/Functions/ImageFunction/flip_x), [!param](/Functions/ImageFunction/flip_y), and
[!param](/Functions/ImageFunction/flip_z)---which may be set in any combination.

## Image Dimensions

By default, the image actual physical dimensions are set to the dimensions of the mesh. However, it
is possible to set the dimensions of the image independently from using the `origin` and `dimensions`
input parameters.

This allows for flexibility to how the `ImageFunction` is utilized. For example, a mesh could be
defined to domain that is smaller than the actual image. Thus, if the `ImageFunction` dimensions are
set to the larger domain, the mesh would only sample some portion of the image. Effectively, this
feature can work on a cropped image, without needing to create a separate cropped image.


## Supported File Types id=file-types

Currently, two types of files are supported \*.tif and \*.png. However, \*.tif files often do not
read correctly with VTK, depending on the format of the file. So, if you experience problems reading
\*.tif files it may require changing the format to \*.png. This can easily be done with any number of
tools with [ImageMagick](http://www.imagemagick.org) being one of the most powerful.

!listing image_function/image_3d.i block=Functions


!syntax parameters /Functions/ImageFunction

!syntax inputs /Functions/ImageFunction

!syntax children /Functions/ImageFunction
