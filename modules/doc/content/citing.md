# Citing MOOSE

## Framework

If you use MOOSE for your publication, please cite the following:

```
@article{permann2020moose,
title = {{MOOSE}: Enabling massively parallel multiphysics simulation},
journal = {{SoftwareX}},
volume = {11},
pages = {100430},
year = {2020},
issn = {2352-7110},
doi = {https://doi.org/10.1016/j.softx.2020.100430},
url = {http://www.sciencedirect.com/science/article/pii/S2352711019302973},
author = {Cody J. Permann and Derek R. Gaston and David Andr{\v{s}} and Robert W. Carlsen and Fande Kong and Alexander D. Lindsay and Jason M. Miller and John W. Peterson and Andrew E. Slaughter and Roy H. Stogner and Richard C. Martineau},
keywords = {Framework, Finite-element, Parallel, Multiphysics, Multiscale},
}
```

Another reference you may consider for a good demonstration problem:

```
@article{gaston2015physics,
  title = {Physics-based multiscale coupling for full core nuclear reactor simulation},
  author = {Derek R. Gaston and Cody J. Permann and John W. Peterson and
            Andrew E. Slaughter and David Andr{\v{s}} and Yaqi Wang and Michael
            P. Short and Danielle M. Perez and Michael R. Tonks and Javier
            Ortensi and Ling Zou and Richard C. Martineau},
  journal={Annals of Nuclear Energy},
  volume={84},
  pages={45--54},
  year={2015},
  publisher={Elsevier}
}
```

## Modules

### Navier-Stokes

If you use the incompressible portion of the Navier Stokes module, please cite:

```
@article{peterson2018overview,
  title = {Overview of the incompressible Navier--Stokes simulation capabilities
           in the MOOSE framework},
  author = {John W. Peterson and Alexander D. Lindsay and Fande Kong},
  journal = {Advances in Engineering Software},
  volume = {119},
  pages = {68--92},
  year = {2018},
  publisher = {Elsevier}
}
```

### Tensor Mechanics

If you use the multi-surface plasticity capability, `ComputeMultiPlasticityStress`, of the Tensor Mechanics module (feel free to contact Andy Wilkins if unsure) or if you just want to demonstrate MOOSE's advanced plasticity features, please cite:

```
@article{wilkins_multisurface_plasticity,
  author = {Deepak P. Adhikary and  Chandana Jayasundara and Robert K. Podgorney and Andy H. Wilkins},
  year = {2016},
  month = {01},
  pages = {218--234},
  title = {A robust return-map algorithm for general multisurface plasticity},
  volume = {109},
  journal = {International Journal for Numerical Methods in Engineering},
  doi = {10.1002/nme.5284}
}
```

If you use smoothed multi-surface plasticity, such plasticity models derived from `MultiParameterPlasticityStressUpdate` (`CappedMohrCoulombStressUpdate`, `TensileStressUpdate`, `CappedDruckerPragerStressUpdate`, `CappedWeakPlaneStressUpdate`, etc - feel free to contact Andy Wilkins if unsure) of if you just want to demonstrate MOOSE's advanced plasticity features, please cite the following.

```
@article{wilkins_smooth_plasticity,
  author = {Andy Wilkins and Benjamin W. Spencer and Amit Jain and Bora Gencturk},
  title = {A method for smoothing multiple yield functions},
  journal = {International Journal for Numerical Methods in Engineering},
  volume = {121},
  number = {3},
  pages = {434--449},
  doi = {10.1002/nme.6215},
  year = {2020}
}
```

### Peridynamics

The following papers document the formulations used in the MOOSE Peridynamics module.

The first paper documents the approach used for irregular discretizations and thermo-mechanical coupling:

```
@article{hu_thermomechanical_2018,
	Author = {Hu, Yile and Chen, Hailong and Spencer, Benjamin W. and Madenci, Erdogan},
	Journal = {Engineering Fracture Mechanics},
	Month = jun,
	Pages = {92--113},
	Title = {Thermomechanical peridynamic analysis with irregular non-uniform domain discretization},
	Volume = {197},
	Year = {2018}}
```

The following papers document the stabilization method used for non-ordinary state-based peridynamics in MOOSE:

```
@article{chen_bond-associated_2018,
	Author = {Chen, Hailong},
	Journal = {Mechanics Research Communications},
	Month = jun,
	Pages = {34--41},
	Title = {Bond-associated deformation gradients for peridynamic correspondence model},
	Volume = {90},
	Year = {2018}}

@article{chen_peridynamic_2019,
	Author = {Chen, Hailong and Spencer, Benjamin W.},
	Journal = {International Journal for Numerical Methods in Engineering},
	Month = feb,
	Number = {6},
	Pages = {713--727},
	Title = {Peridynamic bond-associated correspondence model: {Stability} and convergence properties},
	Volume = {117},
	Year = {2019}}
```

### XFEM

The following papers document various aspects of the MOOSE XFEM module.

This paper documents the algorithms used for mesh cutting and partial element integration, and shows applications on several coupled thermal-mechanical problems:

```
@article{jiang_ceramic_2020,
	Author = {Jiang, Wen and Spencer, Benjamin W. and Dolbow, John E.},
	Journal = {Engineering Fracture Mechanics},
	Month = jan,
	Pages = {106713},
	Title = {Ceramic nuclear fuel fracture modeling with the extended finite element method},
	Volume = {223},
	Year = {2020}}
```

This paper documents the moment fitting algorithm that can optionally be used for improved accuracy with MOOSE's XFEM implementation:

```
@article{zhang_modified_2018,
	Author = {Zhang, Ziyu and Jiang, Wen and Dolbow, John E. and Spencer, Benjamin W.},
	Journal = {Computational Mechanics},
	Month = aug,
	Number = {2},
	Pages = {233--252},
	Title = {A modified moment-fitted integration scheme for {X}-{FEM} applications with history-dependent material data},
	Volume = {62},
	Year = {2018}}
```
