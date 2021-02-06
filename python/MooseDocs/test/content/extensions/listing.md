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

!listing prefix=File caption=the =caption= id=file1
caption with prefix and number

Reference to special prefix: [file1].

## Listing with space

!listing!
void function();

void anotherFunction();
!listing-end!

### Test Limited Height

The container for this listing is limited to 92 pixels, vertically:

!listing! max-height=92px
#include <stdio.h>

void
greeting(void)
{
  printf("Hello, World!\n");
}

int
main(int argc, char **argv)
{
  greeting();
}
!listing-end!

## Language

!listing language=c++
void function();

## File Listings

Display a C++ source file (excluding the MOOSE header) and hide the modal link:

!listing framework/src/kernels/Diffusion.C
         link=False
         id=diffusion-c

Display only the `validParams()` definition and limit height to 92 pixels, vertically:

!listing framework/src/kernels/Diffusion.C
         start=InputParameters
         end=::Diffusion
         max-height=92px

### HIT Files

Extract the `[Mesh]` and `[Kernels]` blocks and use a special prefix:

!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i
         block=Mesh Kernels
         prefix=xxxxx
         id=prfx

Extract the `[diff]` block, indent 2 spaces, and add `[AuxKernels]` and `[]` as header and footer:

!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i
         block=Kernels/diff
         indent=2
         header=[AuxKernels]
         footer=[]

Remove the `[test]` block and hide the modal link:

!listing moose/test/tests/kernels/simple_diffusion/tests
         link=False
         remove=test

Remove the `issues` and `design` parameters:

!listing moose/test/tests/kernels/simple_diffusion/tests
         remove=/Tests/test/issues Tests/test/design

Extract the `[BCs]` block then remove the `[left]` block and the `value` parameter:

!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i
         block=BCs
         remove=left BCs/right/value
