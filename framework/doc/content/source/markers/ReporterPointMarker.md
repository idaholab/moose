# ReporterPointMarker

!syntax description /Adaptivity/Markers/ReporterPointMarker

## Description

The `ReporterPointMarker` is a stand-alone marker that marks all
elements as either "inside" or "outside" based on whether or not the element contains a point defined by a vector of coordinates given in a `Reporter`.  

## Example Input Syntax

The mesh domain in the following input file is a 2D square with $x=(0,1)$ and $y=(0,1)$.  The marker labeled "box" gets valid coordinates from the vector coords in the `ConstantReporter`.  For this 2D domain, only points with coord/z=0 vector entries will be found inside the domain.  The final reporter vector entry with coord/z=1 will produce a point outside the domain and will be ignored.  Elements containing a point them are marked with the "REFINE" flag and empty elements are marked "DO_NOTHING".  Alternatively, empty elements could be marked with the "COARSEN" flag to coarsen the mesh if points move out of an element.

In the `ReporterPointMarker` labeled `bad_coord`, the reporter vectors containing coordinates are a different size, triggering an error.  

!listing test/tests/markers/reporter_point_marker/point_marker_test.i block=Adaptivity

!syntax parameters /Adaptivity/Markers/ReporterPointMarker

!syntax inputs /Adaptivity/Markers/ReporterPointMarker

!syntax children /Adaptivity/Markers/ReporterPointMarker
