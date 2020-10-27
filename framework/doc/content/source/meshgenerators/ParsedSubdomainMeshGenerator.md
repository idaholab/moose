# ParsedSubdomainMeshGenerator

!syntax description /Mesh/ParsedSubdomainMeshGenerator

## Example

The desired example mesh is a 1-by-1 2D square which contains Block 1 (a centered 0.8-by-0.8 square) and Block 2 (a 0.4-by-0.4 square located in the bottom left quarter of Block 1). The remaining edge of the square can be Block 0.

The combinatorial expression that defines Block 1 is below.

```
x > 0.1 & x < 0.9 & y > 0.1 & y < 0.9
```

The expression

```
x < 0.5 & y < 0.5
```

can partially define Block 2, but the region outside Block 1 also needs to be excluded. The input file syntax needed to generate this example is shown below.

!listing test/tests/meshgenerators/parsed_subdomain_mesh_generator/parsed_subdomain_mg.i block=Mesh

The final mesh output is:

!media large_media/parsed_subdomain_mesh/parsed_subdomain_mesh.png style=width:75%;float:center

!syntax parameters /Mesh/ParsedSubdomainMeshGenerator

!syntax inputs /Mesh/ParsedSubdomainMeshGenerator

!syntax children /Mesh/ParsedSubdomainMeshGenerator
