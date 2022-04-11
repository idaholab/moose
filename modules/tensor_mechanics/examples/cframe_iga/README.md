# Isogeometric Analysis in MOOSE

This readme illustrates using IGA within the MOOSE framework for a particular example.

To use some of the files in this example a Coreform Cubit license is needed to create the mesh file needed to run this example.
Coreform Cubit is a product released and maintained by Coreform, LLC. 
A free to use Cubit Learn license can be acquired here: https://coreform.com/products/coreform-cubit/free-meshing-software/

To execute the Cubit journal file in batch from the command line use the following command: 

```
coreform_cubit -batch cframe_build.jou
```

Within the journal file there are unique commands that will generate a uspline on the discretized mesh. 
These commands are on lines 34-36.
* Line 35 sets the degree and continuity of the uspline
* As of this writing, the max degree supported by libmesh is 2.
* All continuity must equal p-1 where p is the degree.
* Line 36 constructs the uspline using the geometry as a basis.
* Line 37 fits the uspline that was built to the geometry.

VTK exporting in the MOOSE deck will output paraview visualization files. 
Exodus outputs will contain the results but will not feature the smooth geometry which the analysis was evaluated.
