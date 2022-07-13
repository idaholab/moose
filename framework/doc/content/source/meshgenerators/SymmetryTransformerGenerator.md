# SymmetryTransformerGenerator

!syntax description /Mesh/SymmetryTransformerGenerator

## Overview

The SymmetryTransformerGenerator makes a mirror reflection of a mesh across an arbitrary plane or line (supplied by the user). All input is expected to be three dimensional even if the mesh in question is two dimensional; in such a case, let the z component be 0 (e.g., `extrusion_vector = '1 1 0'`). The values of the components can be floating types, not just integers. 

The user sets the plane that will be reflected over by giving two vectors: a vector that gives the position of any given point on the line/plane from the origin; and, a vector that is normal (aka perpendicular/ orthogonal) to said line/plane. The normal vector establishes the slope of the plane of reflection. 

Internally, SymmetryTransformerGenerator applies a matrix operation to the vectors the user supplied. The matrix that is used assumed that the inputted normal vector is also a unit vector, too (meaning, the length of the vector is 1). However, it is impractical to ask that the user give the x,y, and z components of a perfect normal unit vector. The user would have to calculate the coordiantes to a high precision to prevent arithmetic rounding from calculating a norm (length) of not exactly 1. 

For example, if the user wanted a give a normal unit vector at 45° angle, even if they entered x and y values to the 10th decimal place of precision (0.7071067811), the norm of the inputted vector would be calculated to be 0.99999999987760335. The maximum decimal places of precision the user can enter is limited by the operating system and compiler used to build MOOSE. The user would not be able to enter some extremely high precision number even if they wanted to (on the author’s machine, up to 17 decimals of precision are accepted). 
For this reason, the user is only expected to enter a normal vector, which is automatically converted to a unit normal vector. While unexpected, it is theoretically possible if the user is dealing with very small, precise areas for arithmetic error to be introduced in the calculation of the unit normal vector. In this case, adjust the inputted normal vector.

!alert warning
No treatment is made on the internally-kept order of nodes in the mirrored mesh, so the mirrored mesh will have a negative connectivity/volume!


!syntax parameters /Mesh/SymmetryTransformerGenerator

!syntax inputs /Mesh/SymmetryTransformerGenerator

!syntax children /Mesh/SymmetryTransformerGenerator
