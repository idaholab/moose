# Functional Expansion Tools

A MOOSE module for continuous, mesh-agnostic, high-fidelity, reduced-data MultiApp coupling

## Description

Functional expansions (FXs) are a methodology that represent information as moments of a functional
series [!citep](Flusser2016). This is is related to a Fourier series representation of cyclic
data. Moments are generated via numerical integration for each term in the functional series to
represent the field of interest. These moments can then be used to reconstruct the field in a
separate app [!citep](Wendt2018a,Wendt2017c,Kerby2017).

Currently there are two main flavors of FX coupling available: interface and volumetric.

!alert note title=Nomenclature
In other contexts FXs have been identified using the initialism 'FE'. However, since MOOSE is a
finite-element (FE) code, the initialism 'FX' is used throughout this module to prevent confusion.

!alert warning title=Displaced meshes
This module should only be used with non-displaced meshes.

### Interface Coupling

Interface-based coupling provides for the coupling between physically distinct solutions that share a
common interface. FXs can be used to extract value and/or flux conditions at a boundary in one app,
then be transferred as boundary conditions to the coupled app. These coupled boundaries represent the
common interface between the two solutions.

### Volumetric Coupling

Volumetric-based coupling provides for coupling between solutions that share the same space with
coupled yet separate physics. The field in one app can be collapsed into an FX, then the moment
values are transferred to the other app to be used in its solution.

## Using this module

Set either `FUNCTIONAL_EXPANSION_TOOLS := yes` (or `ALL_MODULES := yes`) in the application
makefile. The following objects will then be available for use:

### AuxKernels

| Name | Description |
| - | - |
| [`FunctionSeriesToAux`](/FunctionSeriesToAux.md) | Expands an FX into the named +AuxVariable+ before the initial nonlinear solve. |


### BCs

| Name | Description |
| - | - |
| [`FXFluxBC`](/FXFluxBC.md) | Provides a *strongly encouraged* FX-based Neumann boundary condition. |
| [`FXValueBC`](/FXValueBC.md) | Provides a *fixed* FX-based Dirichlet boundary condition. |
| [`FXValuePenaltyBC`](/FXValuePenaltyBC.md) | Provides a *strongly encouraged* FX-based Dirichlet boundary condition. |


### Functions

| Name | Description |
| - | - |
| [`FunctionSeries`](/FunctionSeries.md) | The workhorse of the FX tools module. This evaluates the terms of a function series, used both for capturing moments and expanding an FX. All other FX-based objects will depend on a `FunctionSeries` instance. |


### Kernels

Although there are no kernels directly provided by this module (yet), these three from the framework itself have varying degrees of usefulness:

| Name | Description |
| - | - |
| [`BodyForce`](/BodyForce.md) | May be used to couple a `FunctionSeries` object directly to a solution, instead of using `FunctionSeriesToAux` and `CoupledForce`. If the `BodyForce` approach is used it is highly recommended to set `enable_cache = true` for the associated `FunctionSeries` object. |
| [`CoupledForce`](/CoupledForce.md) | Couples an +AuxVariable+ or +Variable+ to the solution of another +Variable+. Useful in conjunction with `FunctionSeriesToAux`.
| [`NullKernel`](/NullKernel.md) | May be required for use in situation where no +Variables+ or +Kernels+ are needed. This may occur, for example, in an app that uses only the recommended `FunctionSeriesToAux`+`CoupledForce` approach. |


### Transfers

| Name | Description |
| - | - |
| [`MultiAppFXTransfer`](/MultiAppFXTransfer.md) | This transfers the FX coefficients, or moments, between named FX objects in the multi and sub apps. Supported objects that contain coefficients for transferring are instances of `FunctionSeries` and any `FX...UserObject`.|


### UserObjects

| Name | Description |
| - | - |
| [`FXBoundaryFluxUserObject`](/FXBoundaryFluxUserObject.md) | Captures the moments of an FX representing the flux at a boundary. |
| [`FXBoundaryValueUserObject`](/FXBoundaryValueUserObject.md) | Captures the moments of an FX representing the value at a boundary. |
| [`FXVolumeUserObject`](/FXVolumeUserObject.md) | Captures the moments of an FX representing the field value over a volume. |

## Examples

Please refer to the [examples](/examples.md) included with the module for how these objects can be used.


## Supported Functional Series

This module currently supports FXs based on the 1D Legendre and 2D Zernike polynomial series. From
these can be constructed 1D, 2D, or 3D Cartesian basis sets (Legendre only), or 3D cylindrical
(Legendre + Zernike) basis sets. Nonseparable series, i.e. with fully-convolved cross terms, are used
in this implementation. Examples for selecting FXs are shown in
[`FunctionSeries`](/FunctionSeries.md).

Additional functional series, polynomial or otherwise, can be added by inheriting from the
`SingleSeriesBasisInterface` class (found in the `utils/` folder). The composite series (currently
the `Cartesian` and `CylindricalDuo` classes) will then need updated to support the newly-available
series.

Additional composite series, such as may be suitable for spherical or shell data, can be implemented
by inheriting from the `CompositeSeriesBasisInterface`. The `FunctionSeries` class will need updated
to support the newly-available series.


## Caveats

1. FXs are not recommended for spanning spaces with discontinuities [!citep](Ellis2017b).
   - One example would be a space containing two distinct materials with significantly different properties
   - Instead, using multiple FXs, each over its own region of continuity, is the recommended approach
2. Increasing the order of an FX does not always result in an improved representation. Numerical integration of the FX moment can yield large errors if not enough quadrature points are included [!citep](Griesheimer2005a).

!bibtex bibliography

## TODO

- Investigate the implementation of using `NearestPointBase` approach to easily equip multiple FXs
- Implement a +Materials+-derived FX-based class that can provide continuous material properties
- Implement a +Kernel+-derived class, a la `BodyForce`, that automatically sets `enable_cache = true` for the associated `FunctionSeries` object
- Add an error check in `MultiAppFXTransfer` for multiple objects of the same name but different type, i.e. if there are both a +Function+ and +UserObject+ with the same name (or other object types as they are added)
- Implement support in `MutableCoefficientsInterface` for multiple sets of FX coefficients
- Implement support for various types of FX derivations
  - Separable series
  - Various orthonormalizations
- Add more functional series
  - Fourier
  - Annular Zernike (0 < r <= 1)
  - Shell (r = 1) for 3D cylindrical boundary conditions (Zernike-based?)
  - Spherical harmonics + spherical composite series
- Add check to ensure we are working in an undisplaced mesh context

## Objects, Actions, and Syntax

!syntax complete groups=FunctionalExpansionToolsApp level=3
