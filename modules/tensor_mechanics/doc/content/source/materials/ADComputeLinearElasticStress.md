# ADComputeLinearElasticStress

!syntax description /ADMaterials/ADComputeLinearElasticStress<RESIDUAL>

This material is a version of
[ComputeLinearElasticStress](/materials/ComputeLinearElasticStress.md)  which
uses forward mode automatic differentiation to carry along derivatives of its
material properties for use with the automatic differentiation tensor mechanics
kernels (such as [ADStressDivergence](/kernels/ADStressDivergenceTensors.md)).

!syntax parameters /ADMaterials/ADComputeLinearElasticStress<RESIDUAL>

!syntax inputs /ADMaterials/ADComputeLinearElasticStress<RESIDUAL>

!syntax children /ADMaterials/ADComputeLinearElasticStress<RESIDUAL>

!bibtex bibliography
