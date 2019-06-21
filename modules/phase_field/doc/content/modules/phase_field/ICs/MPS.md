# Use Maximal Poisson-Disk Sampling (MPS) to generate grain centroids

Random close-packing (RCP) structure can be realized by a spatial sampling process known as Maximal Poisson-Disk Sampling (MPS). For the RCP structure, the average aspect ratio of each Voronoi cell is approximately one. The RCP Voronoi structure provides an equiaxed grain structure.

The MPS algorithm is described in [!cite](Ebeida_2012) and it is implemented in [MeshingGenie](https://github.com/trilinos/Trilinos/tree/master/packages/meshinggenie) package in [Trilinos](https://trilinos.org). MeshingGenie will generate a list of grain centroids as a text file that can then be read into the PolycrystalVoronoi initial condition.

## Build instruction for standalone version of MeshingGenie

- Download release version of [Trilinos](https://github.com/trilinos/Trilinos/releases/tag/trilinos-release-12-12-1) (Note: please do not git clone!)
- Run `cmake .` in the Trilinos/packages/meshinggenie/src/standalone/ directory
- Run `make`. Executable file named MeshingGenie is located in the Trilinos/packages/meshinggenie/src/standalone/app directory

## Run MeshingGenie to generate the centroids of grains

To run MeshingGenie, type `./MeshingGenie input.dat`. In the input file `input.dat`, the first line is "p", which indicates MPS. The second line should be in the format of "D R S N_bp N_bf", where D is the dimension, R the distribution radius, S the random seed, N_bp the number of boundary points and N_bf the number of boundary
faces. The next N_bp lines are the x, y, and z of the boundary points in order. Each entry of the next N_bf lines describes a triangularized boundary face using the indices of the boundary points listed in an ordered direction (outwards normal using the right hand rule).

An example of 2d input file `2d.dat` is given below

```text
p
2 0.3 10 4 4
0 0
1 0
1 1
0 1
0 1
1 2
2 3
3 0
```

and 3d input file `3d.dat` is given below

```text
p
3 0.3 10 8 12
0 0 0
1 0 0
1 1 0
0 1 0
0 0 1
1 0 1
1 1 1
0 1 1
0 3 2
0 2 1
1 2 5
2 6 5
0 1 5
0 5 4
0 4 3
3 4 7
3 7 6
3 6 2
5 6 7
5 7 4
```

The output file containing centroids is stored in `maximal_sample.dat`. The first line contains the dimension, distribution radius and number of grains. An example of output file `maximal_sample.dat` is given below

```text
2 0.3 5
0.232307832746474 0.452347042281606
0.310283982183736 0.848418291915013
0.561253436179246 0.534434874769859
0.665658262188368 0.214715684556803
0.714381419175506 0.826132085262111
```

## Use the centroids to generate Voronoi tessellation in Phase field

The [`PolycrystalVoronoi`](/PolycrystalVoronoi.md) can read grain centroids from a text file and performs a Voronoi tesslation to produce a grain structure. The syntax is

```text
[UserObjects]
  [./voronoi]
    type = PolycrystalVoronoi
    grain_num = 5
    file_name = 'grains.txt'
  [../]
[]
```

The `grains.txt` contains the grain centroids that can be taken from `maximal_sample.dat`. An example of text file `grains.txt` is given below

```text
x                 y
0.232307832746474 0.452347042281606
0.310283982183736 0.848418291915013
0.561253436179246 0.534434874769859
0.665658262188368 0.214715684556803
0.714381419175506 0.826132085262111
```

!bibtex bibliography

