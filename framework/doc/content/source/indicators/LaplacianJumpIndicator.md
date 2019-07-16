# LaplacianJumpIndicator

!syntax description /Adaptivity/Indicators/LaplacianJumpIndicator

## Description

The LaplacianJumpIndicator object computes the error as computed by the change in the
Laplacian of a variable across element interfaces.

!alert warning title=LaplacianJumpIndicator requires second derivatives
The Laplacian ($\nabla^2 u$ or $\nabla\cdot\nabla u$) operator requires second derivaties with
respect to the spacial dimensions. As such, the selected finite elements must be at least
second order for the calculation to be valid.

## Example Input File Syntax

The following code snippet demonstrates the use of the LaplacianJumpIndicator object within a
typical input file.

!listing biharmonic.i block=Adaptivity

!syntax parameters /Adaptivity/Indicators/LaplacianJumpIndicator

!syntax inputs /Adaptivity/Indicators/LaplacianJumpIndicator

!syntax children /Adaptivity/Indicators/LaplacianJumpIndicator
