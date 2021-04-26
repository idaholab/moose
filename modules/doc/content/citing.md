# Citing MOOSE

This page lists the references to use if you are using MOOSE for in your publication. For a
list of publications that have cited MOOSE, please refer to the [publications.md].

## MOOSE

For all publications that use MOOSE or a MOOSE-based application please cite the following.

```tex
@article{permann2020moose,
   title = {{MOOSE}: Enabling massively parallel multiphysics simulation},
  author = {Cody J. Permann and Derek R. Gaston and David Andr{\v{s}} and Robert W. Carlsen and Fande
            Kong and Alexander D. Lindsay and Jason M. Miller and John W. Peterson and Andrew
            E. Slaughter and Roy H. Stogner and Richard C. Martineau},
    year = {2020},
 journal = {{SoftwareX}},
  volume = {11},
   pages = {100430},
    issn = {2352-7110},
     doi = {https://doi.org/10.1016/j.softx.2020.100430},
     url = {http://www.sciencedirect.com/science/article/pii/S2352711019302973},
keywords = {Framework, Finite-element, Parallel, Multiphysics, Multiscale}
}
```

## MultiApp System

If your application uses the MultiApp system, please also cite the following.

```
@article{gaston2015physics,
    title = {Physics-based multiscale coupling for full core nuclear reactor simulation},
   author = {Derek R. Gaston and Cody J. Permann and John W. Peterson and Andrew E. Slaughter and
             David Andr{\v{s}} and Yaqi Wang and Michael P. Short and Danielle M. Perez and Michael
             R. Tonks and Javier Ortensi and Ling Zou and Richard C. Martineau},
     year = {2015},
  journal = {Annals of Nuclear Energy},
   volume = {84},
    pages = {45--54},
publisher = {Elsevier}
}
```

## Automatic Differentiation

If your application uses automatic differentiation, please also cite the following.

```
@article{lindsay2021automatic,
  title={Automatic Differentiation in MetaPhysicL and Its Applications in MOOSE},
  author={Lindsay, Alexander and Stogner, Roy and Gaston, Derek and Schwen, Daniel and Matthews,
  Christopher and Jiang, Wen and Aagesen, Larry K and Carlsen, Robert and Kong, Fande and Slaughter,
  Andrew and others},
  journal={Nuclear Technology},
  pages={1--18},
  year={2021},
  publisher={Taylor \& Francis}
}
```

## Testing and Documentation

If you are utilizing [!ac](CIVET) for testing or [MooseDocs](python/MooseDocs/index.md) for
documentation, please also cite the following.

```
@article{slaughter2021continuous,
author    = {Andrew E. Slaughter and Cody J.Permann and Jason M. Miller and Brian K. Alger and Stephen R. Novascone},
title     = {Continuous Integration, In-Code Documentation, and Automation for Nuclear Quality Assurance Conformance},
journal   = {Nuclear Technology},
volume    = {0},
number    = {0},
pages     = {1--8},
year      = {2021},
publisher = {Taylor & Francis},
doi       = {10.1080/00295450.2020.1826804},
url       = {https://doi.org/10.1080/00295450.2020.1826804}
}
```

## Modules

If you are using a physics module for your application, please cite the appropriate references
as listed here.

### Navier-Stokes

If you use the incompressible portion of the Navier-Stokes module, please cite:

```
@article{peterson2018overview,
    title = {Overview of the incompressible Navier--Stokes simulation capabilities in the MOOSE
             framework},
   author = {John W. Peterson and Alexander D. Lindsay and Fande Kong},
     year = {2018},
  journal = {Advances in Engineering Software},
   volume = {119},
    pages = {68--92},
publisher = {Elsevier}
}
```

### Tensor Mechanics

If you use the multi-surface plasticity capability, `ComputeMultiPlasticityStress`, of the Tensor Mechanics module (feel free to contact Andy Wilkins if unsure) or if you just want to demonstrate MOOSE's advanced plasticity features, please cite:

```
@article{adhikary2016robust,
  title = {A robust return-map algorithm for general multisurface plasticity},
 author = {Deepak P. Adhikary and  Chandana Jayasundara and Robert K. Podgorney and Andy H. Wilkins},
   year = {2016},
journal = {International Journal for Numerical Methods in Engineering},
  month = {01},
  pages = {218--234},
 volume = {109},
    doi = {10.1002/nme.5284}
}
```

If you use smoothed multi-surface plasticity, such plasticity models derived from `MultiParameterPlasticityStressUpdate` (`CappedMohrCoulombStressUpdate`, `TensileStressUpdate`, `CappedDruckerPragerStressUpdate`, `CappedWeakPlaneStressUpdate`, etc - feel free to contact Andy Wilkins if unsure) of if you just want to demonstrate MOOSE's advanced plasticity features, please cite the following.

```
@article{wilkins2020method,
  title = {A method for smoothing multiple yield functions},
 author = {Andy Wilkins and Benjamin W. Spencer and Amit Jain and Bora Gencturk},
   year = {2020},
journal = {International Journal for Numerical Methods in Engineering},
 volume = {121},
 number = {3},
  pages = {434--449},
    doi = {10.1002/nme.6215}
}
```

### Peridynamics

The following papers document the formulations used in the MOOSE Peridynamics module.

The first paper documents the approach used for irregular discretizations and thermo-mechanical coupling:

```
@article{hu2018thermomechanical,
  title = {Thermomechanical peridynamic analysis with irregular non-uniform domain discretization},
 author = {Hu, Yile and Chen, Hailong and Spencer, Benjamin W. and Madenci, Erdogan},
   year = {2018},
journal = {Engineering Fracture Mechanics},
  month = {June},
  pages = {92--113},
 volume = {197}
}
```

The following papers document the stabilization method used for non-ordinary state-based peridynamics in MOOSE:

```
@article{chen2018bond,
  title = {Bond-associated deformation gradients for peridynamic correspondence model},
 author = {Chen, Hailong},
   year = {2018},
journal = {Mechanics Research Communications},
  month = {June},
  pages = {34--41},
 volume = {90}
}

@article{chen2019peridynamic,
  title = {Peridynamic bond-associated correspondence model: {Stability} and convergence properties},
 author = {Chen, Hailong and Spencer, Benjamin W.},
journal = {International Journal for Numerical Methods in Engineering},
   year = {2019},
  month = {February},
 number = {6},
  pages = {713--727},
 volume = {117}
}
```

### Porous Flow

The [following paper](https://doi.org/10.21105/joss.02176) provides an overview of the MOOSE Porous Flow module.

```
@article{Wilkins2020,
  doi = {10.21105/joss.02176},
  url = {https://doi.org/10.21105/joss.02176},
  year = {2020},
  publisher = {The Open Journal},
  volume = {5},
  number = {55},
  pages = {2176},
  author = {Andy Wilkins and Christopher P. Green and Jonathan Ennis-King},
  title = {PorousFlow: a multiphysics simulation code for coupled problems in porous media},
  journal = {Journal of Open Source Software}
}
```

### XFEM

The following papers document various aspects of the MOOSE XFEM module.

This paper documents the algorithms used for mesh cutting and partial element integration, and shows applications on several coupled thermal-mechanical problems:

```
@article{jiang2020ceramic,
  title = {Ceramic nuclear fuel fracture modeling with the extended finite element method},
 author = {Jiang, Wen and Spencer, Benjamin W. and Dolbow, John E.},
journal = {Engineering Fracture Mechanics},
   year = {2020}
  month = {January},
  pages = {106713},
 volume = {223}
}
```

This paper documents the moment fitting algorithm that can optionally be used for improved accuracy with MOOSE's XFEM implementation:

```
@article{zhang2018modified,
  title = {A modified moment-fitted integration scheme for {X}-{FEM} applications with
           history-dependent material data},
 author = {Zhang, Ziyu and Jiang, Wen and Dolbow, John E. and Spencer, Benjamin W.},
journal = {Computational Mechanics},
   year = {2018},
  month = {August},
 number = {2},
  pages = {233--252},
 volume = {62}
}
```
