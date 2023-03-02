# RadialAverage

!syntax description /UserObjects/RadialAverage

Given a material property and a radius for averaging, the RadialAverage object
computes the spatial average value of the property over the radius.

## Applications

This can be used for nonlocal damage models in the `TensorMechanics` module
where the damage_index that is used for computing the damage stress is average
over a certain radius $r_0$. This can help alleviate mesh sensitivity in certain
cases. This can be accomplished by running the RadialAverage object on a local
damage material property. Then using the `NonlocalDamage` model in conjunction
with the `ComputeDamageStress` the damage index used for updating the stress is
averaged over a certain radius.

## Design

The RadialAverage user object is derived from `ElementUserObject` and
works in two stages.

1. In the element loop (in the `execute()` method) a list of all quadrature
   points, their locations, indices, and selected material property value is compiled.

2. In the `finalize()` method

    1. the list is communicated in parallel

    2. a KD-tree is filled with all quadrature point entries (utilizing the
        [nanoflann](https://github.com/jlblancoc/nanoflann) library bundled with
        libMesh)

    3. a loop over all QPs is performed and at each QP a (`radius`)
        radius search in the KD-tree is performed

    4. the results from the search are used to spatially averaged


The [!param](/UserObjects/RadialAverage/weights) parameter determines the distance
based weight function to be used in the averaging process. `constant` assigns an equal weight
to all material points, `linear` weights each point by $r_0-r$ (a linear fall-off with distance),
and `cosine` weights each point by $1+\cos (\frac{r}{r_0} \pi)$.

!alert note
The weights mentioned above currently do not take into account the quadrature point weight.

!syntax parameters /UserObjects/RadialAverage

!syntax inputs /UserObjects/RadialAverage

!syntax children /UserObjects/RadialAverage

!bibtex bibliography
