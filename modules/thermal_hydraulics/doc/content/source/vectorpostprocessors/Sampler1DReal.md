# Sampler1DReal

!syntax description /VectorPostprocessors/Sampler1DReal

This object has similar capabilities to the [ElementMaterialSampler.md], but is
implemented differently. It creates its own standalone loop on elements in the mesh
to gather the material properties.

!alert note
To sample AD material properties, use `ADSampler1DReal`.

!syntax parameters /VectorPostprocessors/Sampler1DReal

!syntax inputs /VectorPostprocessors/Sampler1DReal

!syntax children /VectorPostprocessors/Sampler1DReal

!bibtex bibliography
