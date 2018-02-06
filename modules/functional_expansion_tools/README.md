# Functional Expansion Tools
###### A MOOSE module for continuous, mesh-agnostic, high-fidelity, reduced-data MultiApp coupling
#
#
#
### Description
Functional expansions (FXs) are a methodology that represent information as moments of a functional series. This is is related to a Fourier series representation of cyclic data. Moments are generated via numerical integration for each term in the functional series to represent the field of interest. These moments can then be used to reconstruct the field in a separate app.

Currently there are two main flavors of FX coupling available: interface and volumetric.

In other contexts FXs have been identified using the initialism 'FE'. However, since MOOSE is a finite-element (FE) code, the initialism 'FX' is used throughout this module.

##### Interface Coupling
Interface-based coupling provides for the coupling between physically distinct solutions that share a common interface. FXs can be used to extract value and/or flux conditions at a boundary in one app, then be transferred as boundary conditions to the coupled app. These coupled boundaries represent the common interface between the two solutions.

##### Volumetric Coupling
Volumetric-based coupling provides for coupling between solutions that share the same space with coupled yet separate physics. The field in one app can be collapsed into an FX, then the moment values are transferred to the other app to be used in its solution.

### Using this module
Set either `ALL_MODULES := yes` or ` FUNCTIONAL_EXPANSION_TOOLS := yes` in the application makefile. The following objects will then be available for use:

##### AuxKernels
#
| Name | Description |
| ------ | ------ |
| `FunctionSeriesToAux` | Expands an FX into the named **AuxVariable** before the initial nonlinear solve. |

##### BCs
#
| Name | Description |
| ------ | ------ |
| `FXFluxBC` | Provides a *strongly encouraged* FX-based Neumann boundary condition. |
| `FXValueBC` | Provides a *fixed* FX-based Dirichlet boundary condition. |
| `FXFluxBC` | Provides a *strongly encouraged* FX-based Dirichlet boundary condition. |

##### Functions
#
| Name | Description |
| ------ | ------ |
| `FunctionSeries` | The workhorse of the FX tools module. This evaluates the terms of a function series, used both for capturing moments and expanding an FX. All other FX-based objects will depend on a `FunctionSeries` instance. |

##### Kernels
Although there are no kernels directly provided by this module (yet), these three have varying degrees of usefulness:
| Name | Description |
| ------ | ------ |
| `BodyForce` | May be used to couple a `FunctionSeries` object directly to a solution, instead of using `FunctionSeriesToAux` and `CoupledForce`. If the `BodyForce` approach is used it is highly recommended to set `enable_cache = true` for the associated `FunctionSeries` object. |
| `CoupledForce` | Couples an **AuxVariable** or **Variable** to the solution of another **Variable**. Useful in conjunction with `FunctionSeriesToAux`.
| `NullKernel` | May be required for use in situation where no **Variables** or **Kernels** are needed. This may occur, for example, in an app that uses only the recommended `FunctionSeriesToAux`+`CoupledForce` approach. |

##### Transfers
#
| Name | Description |
| ------ | ------ |
| `MultiAppFXTransfer` | This transfers the FX coefficients, or moments, between named FX objects in the multi and sub apps. Supported objects that contain coefficients for transferring are instances of `FunctionSeries` and any `FX...UserObject`. (Note: `MultiAppFXTransfer` is actually a typedef of `MultiAppMutableCoefficientsTransfer`).|

##### UserObjects
#
| Name | Description |
| ------ | ------ |
| `FXBoundaryFluxUserObject` | Captures the moments of an FX representing the flux at a boundary. |
| `FXBoundaryValueUserObject` | Captures the moments of an FX representing the value at a boundary. |
| `FXVolumeUserObject` | Captures the moments of an FX representing the field value over a volume. |


Please refer to the examples for how these objects are used.

### Supported Functional Series
This module currently supports FXs based on the 1D Legendre and 2D Zernike polynomial series. From these can be constructed 1D, 2D, or 3D Cartesian basis sets (Legendre only), or 3D cylindrical (Legendre + Zernike) basis sets. Nonseparable series, i.e. with fully-convolved cross terms, are used in this implementation.

Additional functional series, polynomial or otherwise, can be added by inheriting from the `SingleSeriesBasisInterface` class (found in the `utils/` folder). The composite series (currently the `Cartesian` and `CylindricalDuo`classes) will then need updated to support the newly-available series.

Additional composite series, such as may be suitable for spherical or shell data, can be implemented by inheriting from the `CompositeSeriesBasisInterface`. The `FunctionSeries` class will need updated to support the newly-available series.

### Caveats
1) FXs are not recommended for spanning spaces with discontinuities.
   * One example would be a space containing two distinct materials with significantly different properties
   * Instead, using multiple FXs, each over its own region of continuity, is the recommended approach
2) Increasing the order of an FX does not always result in an improved representation. Numerical integration of the FX moment can yield large errors if not enough quadrature points are included.

### Relevant Publications (Newest to Oldest)
* B. WENDT, A. NOVAK, L. KERBY, and P. ROMANO, "Integration of Functional Expansion Methodologies as a MOOSE Module," in *PHYSOR 2018: Reactor Physics paving the way towards more efficient systems,* (2018). submitted
* M. ELLIS, *Methods for Including Multiphsyics Feedback in Monte Carlos Reactor Physics Calculations.* PhD Dissertation, Massachusetts Institute of Technology (2017).
* B. WENDT and L. KERBY, "MultiApp Transfers in the MOOSE Framework based on Functional Expansions." *Transactions of the American Nuclear Society,* **117**(1), pp. 735-738 (2017).
* L. KERBY, A. TUMULAK, J. LEPPANEN, and V. VALTAVIRTA, "Preliminary Serpent-MOOSE Coupling and Implementation of Functional Expansion Tallies in Serpent," in *International Conference on Mathematics & Computational Methods Applied to Nuclear Science and Entineering (M&C 2017),* (2017)
* J. FLUSSER, T. SUK, and B. ZITOV, *2D & 3D image analysis by moments.* John Wiley & Sons, Inc. (2016)
* D. GRIESHEIMER, *Functional Expansion Tallies for Monte Carlo Simulations.* PhD Dissertation, University of Michigan (2005).

### TODO
* Investigate the implementation of using `NearestPointBase` approach to easily equip multiple FXs
* Implement a **Materials**-derived FX-based class that can provide continuous material properties
* Implement a **Kernel**-derived class, a la `BodyForce`, that automatically sets `enable_cache = true` for the associated `FunctionSeries` object
* Add an error check in `MultiAppMutableCoefficientsTransfer` for multiple objects of the same name but different type, i.e. if there are both a **Function** and **UserObject** with the same name (or other object types as they are added)
* Implement support in `MutableCoefficientsInterface` for multiple sets of FX coefficients
* Implement support for various types of FX derivations
  * Separable series
  * Various orthonormalizations
* Add more functional series
  * Fourier
  * Annular Zernike (0 < r <= 1)
  * Shell (r = 1) for 3D cylindrical boundary conditions (Zernike-based?)
  * Spherical harmonics + spherical composite series
* Add check to ensure we are working in an undisplaced mesh context
