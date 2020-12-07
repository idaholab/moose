# Listing Extension

## Test Numbering

!listing id=one
One

!include listing_include.md

!listing id=three
Three

You can reference listings: [three].

## Test Captions

!listing
no caption

!listing caption=the +caption+
caption with inline content

!listing caption=the =caption= id=four
caption with number

## Listing with space

!listing!
void function();

void anotherFunction();
!listing-end!

## Language

!listing language=c++
void function();


## File Listing

!listing framework/src/kernels/Diffusion.C link=False id=diffusion-c

!listing framework/src/kernels/Diffusion.C link=True start=template end=Diffusion::

!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i block=Mesh Kernels prefix=xxxxx

!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels/diff indent=4 header=[AuxKernels] footer=[./]

## Remove input file lines

Hide the `issues` parameter in the following listing:

!listing moose/test/tests/kernels/simple_diffusion/tests
         remove=/Tests/test/issues

!listing moose/test/tests/kernels/simple_diffusion/tests
         remove=/Tests/test
