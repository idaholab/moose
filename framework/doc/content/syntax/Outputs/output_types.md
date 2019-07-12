| Short-cut | Sub-block ("type=") | Description |
| :- | :- | :- |
| console    | Console | Writes to the screen and optionally a file |
| exodus     | Exodus  | The most common,well supported, and controllable output type |
| vtk        | VTK     | Visualization Toolkit format, requires `--enable-vtk` when building libMesh |
| gmv        | GMV     | General Mesh Viewer format |
| nemesis    | Nemesis | Parallel ExodusII format |
| tecplot    | Tecplot | Requires `--enable-tecplot` when building libMesh |
| xda        | XDA     | libMesh internal format (ascii) |
| xdr        | XDR     | libMesh internal format (binary) |
| csv        | CSV     | Comma separated scalar values |
| gnuplot    | GNUPlot | Only support scalar outputs |
| checkpoint | Checkpoint | MOOSE internal format used for restart and recovery |
