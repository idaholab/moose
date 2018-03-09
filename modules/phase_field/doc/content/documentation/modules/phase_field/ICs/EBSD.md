# Reading EBSD Data

To read experimental [electron backscatter diffraction (EBSD)](https://en.wikipedia.org/wiki/Electron_backscatter_diffraction)
data three components are needed

- [`EBSDMesh`](/EBSDMesh.md) Mesh object
- [`EBSDReader`](/EBSDReader.md) UserObjcet
- Initial conditions (such as the `ReconVarIC` action provides)

!media media/phase_field/EBSDReader_stress.png
       style=width:49%;margin-right:1%;float:left;
       caption=Reconstructed microstructure with stress, created using the combined module
               example file `EBSD_reconstruction_grain_growth_mech.i`

!media media/phase_field/EBSDReader_example.png
       style=width:49%;margin-left:1%;float:right;
       caption=Reconstructed microstructure using EBSDReader, created using the phase_field module
               example file `IN100-111grn.i`

## Mesh

The mesh is generated from the EBSD information in the specified EBSD data file to get an optimal
reconstruction of the data. This is accomplished in the mesh block using the
[`EBSDMesh`](/EBSDMesh.md) type. The mesh is created with one node per data point in the
EBSD data file. If you wish to use mesh adaptivity and allow the mesh to get coarser during the
simulation, the `uniform_refine` parameter is used to set how many times the mesh can be coarsened.
The block takes the form:

!listing modules/phase_field/examples/ebsd_reconstruction/IN100-111grn.i start=Mesh end=GlobalParams

## EBSD Reader UserObject

The UserObject reads in the data file, using the name supplied in the mesh block,
and stores a data object with the local data at each material point as well as the
average data about each grain. The block syntax is very simple:

!listing modules/phase_field/examples/ebsd_reconstruction/IN100-111grn.i start=UserObjects end=Variables

## Applying Initial Conditions

The initial condition for the variables is set from the EBSD data. There are three
possible use cases summarized below.

### Case 1: Create grain structure from the grain numbers in the EBSD data, ignoring the phase number

A grain structure is created from the EBSD data by assigning initial condition
values for order parameters. Many more grains can be represented than the number
of order parameters. The required blocks are

!listing modules/phase_field/test/tests/reconstruction/1phase_reconstruction.i start=Mesh end=ENDDOC

### Case 2: Initialize a variable from a specific phase number in the EBSD data, ignoring the grain numbers

Here, the value for a single variable is initialized from the EBSD data corresponding
to a single phase number. The required blocks are

!listing modules/phase_field/test/tests/reconstruction/2phase_reconstruction.i start=Mesh end=ENDDOC

### Case 3: Create an initial grain structure from the EBSD data only corresponding to one phase number
Here, the grain and phase numbers are used. The order parameters are initialized
from the EBSD data, but only using those grains with a given phase number.

!listing modules/phase_field/test/tests/reconstruction/2phase_reconstruction2.i start=Mesh end=ENDDOC

## Using EBSD Crystal Info

The `EBSDReader` local grid data is extracted using the `getData(Point)` function call,
where you pass in location of the point where you want the data. The available data
that can be extracted for a given point is

- `phi1` - The first Euler angle $\phi_1$
- `phi` - The second Euler angle $\Phi$
- `phi2` - The third Euler angle $\phi_2$
- `grain` - The index of the grain
- `phase` - The index of the phase
- `symmetry` - The symmetry class (from TSL)

An example of using this function is shown here

```cpp
const EBSDReader::EBSDData & d = _ebsd_reader.getData(p);
_euler_angles(0) = d.phi1;
_euler_angles(1) = d.phi;
_euler_angles(2) = d.phi2;
```

The EBSDReader average grain data is extracted using the `getAvgData(unsigned int)`
function call, where you pass in the grain number for which you want the data.
The available data that can be extracted

- `phi1` - The average first Euler angle $\phi_1$
- `phi` - The average second Euler angle $\Phi$
- `phi2` - The average third Euler angle $\phi_2$
- `phase` - The index of the phase of the grain
- `symmetry` - The symmetry class (from TSL)
- `p` - Point with centroid location

An example of using this function is show here, taken from ReconVarIC

```cpp
const EBSDReader::EBSDAvgData & d = _ebsd_reader.getAvgData(grn_index);
_centerpoints[gr] = d.p;
```

## Plotting Color Representation of Crystal Orientations

!media media/phase_field/RGB_plot.png
       style=width:30%;margin-left:20px;float:right;
       caption=Reconstructed microstructure with the color representation of the inverse polefigure
               description of the crystyal orientations. Image created using the phase_field module
               example file `IN100-111grn.i`.

It is common to use an inverse pole figure representation of the crystal orientations
to color the grains to represent EBSD data. To simplify the comparison with experiments,
MOOSE has a tool for outputting color values for the inverse pole figure representation
that can then be visualized using Paraview. The spatially varying red, green, and
blue (RGB) values are outputted as auxvariables that are automatically read by
Paraview as a vector.

Two `Auxkernels` can be used to output the RGB values. The first,
[EulerAngleProvider2RGBAux](http://mooseframework.org/docs/doxygen/modules/classEulerAngleProvider2RGBAux.html)
is the simplest but requires the entire domain to have the same crystal structure. The second,
[EulerAngleVariables2RGBAux](http://mooseframework.org/docs/doxygen/modules/classEulerAngleVariables2RGBAux.html)
requires various other auxvariables that contain the Euler angles, the crystal structure,
and the phase number.

The easiest way of outputting the values is to use a custom action block in the
input file that is available. The syntax is

```text
[Modules]
  [./PhaseField]
    [./EulerAngles2RGB]
      crystal_structure = cubic
      euler_angle_provider = ebsd
      grain_tracker = grain_tracker
    [../]
  [../]
[]
```

We recommend you plot the colors using Paraview. The EulerAngle2RGB action will create three
auxvariables with default names `RGB_x`, `RGB_y`, and `RGB_z`. Paraview will automatically create a
vector variable of name `RGB_`. To correctly represent the colors,

1.  Select `RGB_` as the visualization variable.
2.  In the properties section with the advanced properties toggled on, uncheck `Map Scalars` under `Scalar Coloring`.
