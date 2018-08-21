# chigger.graphs

The graphs package is used to create 2D scatter or line plots within
a window, this window can include other 2D and 3D rendererd objects.

Graphs are created using a Graph object, each graph object can contain
multiple line objects. For details on each of these objects please refer
to the pages below.

- [`Graph`](/Graph.md)
- [`Line`](/Line.md)

# Example

!listing line_sample/line_sample_elem.py
         start=import
         id=graph-example
         caption=Example script using a `Graph` and 3D rendering within the same window.

!media chigger/line_sample_elem.png
       style=width:30%;margin:auto;
       id=gram-example-output
       caption=Example output from script using a `Graph` and 3D rendering within the same window.
