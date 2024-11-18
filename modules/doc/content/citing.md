# Citing MOOSE

This page lists the references to use if you are using MOOSE for in your publication. For a
list of publications that have cited MOOSE, please refer to the [publications.md].

## MOOSE

For all publications that use MOOSE or a MOOSE-based application please cite the following.

```tex
@article{giudicelli2024moose,
   title = {3.0 - {MOOSE}: Enabling massively parallel multiphysics simulations},
   author = {Guillaume Giudicelli and Alexander Lindsay and Logan Harbour and Casey Icenhour and
             Mengnan Li and Joshua E. Hansel and Peter German and Patrick Behne and Oana Marin and
             Roy H. Stogner and Jason M. Miller and Daniel Schwen and Yaqi Wang and Lynn Munday and
             Sebastian Schunert and Benjamin W. Spencer and Dewen Yushu and Antonio Recuero and
             Zachary M. Prince and Max Nezdyur and Tianchen Hu and Yinbin Miao and
             Yeon Sang Jung and Christopher Matthews and April Novak and Brandon Langley and
             Timothy Truster and Nuno Nobre and Brian Alger and David Andr{\v{s}} and
             Fande Kong and Robert Carlsen and Andrew E. Slaughter and John W. Peterson and
             Derek Gaston and Cody Permann},
    year = {2024},
 journal = {{SoftwareX}},
  volume = {26},
   pages = {101690},
    issn = {2352-7110},
     doi = {https://doi.org/10.1016/j.softx.2024.101690},
     url = {https://www.sciencedirect.com/science/article/pii/S235271102400061X},
keywords = {Framework, Finite-element, Finite-volume, Parallel, Multiphysics, Multiscale},
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

## Machine Learning Integration using Libtorch

If you are using Libtorch APIs within MOOSE, please consider citing:

```
@article{german2023enabling,
  title={Enabling scientific machine learning in MOOSE using Libtorch},
  author={German, P{\'e}ter and Yushu, Dewen},
  journal={SoftwareX},
  volume={23},
  pages={101489},
  year={2023},
  publisher={Elsevier}
}
```

## Modules

If you are using a physics module for your application, please cite the appropriate references
as listed here.

### Navier-Stokes

The following document summarizes the main capabilities available in the Navier-Stokes module:

```
@article{lindsay2023moose,
  title={MOOSE Navier--Stokes module},
  author={Lindsay, Alexander and Giudicelli, Guillaume and German, Peter and Peterson, John and Wang, Yaqi and Freile, Ramiro and Andrs, David and Balestra, Paolo and Tano, Mauricio and Hu, Rui and others},
  journal={SoftwareX},
  volume={23},
  pages={101503},
  year={2023},
  publisher={Elsevier}
}
```

If you use the finite element incompressible portion of the Navier-Stokes module, please considering citing
the following paper as well:

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

For a more detailed summary of the finite volume incompressible,
compressible and weakly compressible implementations in the Navier-Stokes module, please cite:

```
@techreport{,
      title = {NEAMS-TH-CRAB},
     author = {Guillaume L. Giudicelli, Alexander D. Lindsay, Ramiro Freile, Jieun Lee},
       year = {2021},
     number = {INL/EXT-21-62895},
institution = {Idaho National Laboratory}
}
```

### Solid Mechanics

If you use the multi-surface plasticity capability, `ComputeMultiPlasticityStress`, of the Solid Mechanics module (feel free to contact Andy Wilkins if unsure) or if you just want to demonstrate MOOSE's advanced plasticity features, please cite:

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

### Thermal Hydraulics

This paper gives an overview of the MOOSE Thermal Hydraulics module (THM):

```
@article{hansel2024,
  author = {Joshua Hansel and David Andrs and Lise Charlot and Guillaume Giudicelli},
  title = {The {MOOSE} Thermal Hydraulics Module},
  journal = {Journal of Open Source Software},
  publisher = {The Open Journal},
  year = {2024},
  volume = {9},
  number = {94},
  pages = {6146},
  doi = {10.21105/joss.06146},
  url = {https://doi.org/10.21105/joss.06146}
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

The following papers present the governing equations of the MOOSE Porous Flow module, along with discussions of its capabilities and implementation details:

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

@article{Wilkins2021,
  title = {An open-source multiphysics simulation code for coupled problems in porous media},
  journal = {Computers \& Geosciences},
  volume = {154},
  pages = {104820},
  year = {2021},
  issn = {0098-3004},
  doi = {10.1016/j.cageo.2021.104820},
  author = {Andy Wilkins and Christopher P. Green and Jonathan Ennis-King}
}
```

### Geochemistry Module

The following paper introduces the MOOSE Geochemistry module, along with discussions of its capabilities and implementation details:

```
@article{Wilkins2021,
  doi = {10.21105/joss.03314},
  url = {https://doi.org/10.21105/joss.03314},
  year = {2021},
  publisher = {The Open Journal},
  volume = {6},
  number = {68},
  pages = {3314},
  author = {Andy Wilkins and Christopher P. Green and Logan Harbour and Robert Podgorney},
  title = {The MOOSE geochemistry module},
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

### Reactor

The following paper documents functionalities and demonstration of the MOOSE Reactor module.

```
@article{shemon2023reactor,
 author = {Emily Shemon and Yinbin Miao and Shikhar Kumar and Kun Mo and Yeon Sang Jung and Aaron Oaks and Scott Richards and Guillaume Giudicelli and Logan Harbour and Roy Stogner},
  title = {MOOSE Reactor Module: An Open-Source Capability for Meshing Nuclear Reactor Geometries},
journal = {Nuclear Science and Engineering},
 volume = {0},
 number = {0},
  pages = {1-25},
  year  = {2023},
    doi = {10.1080/00295639.2022.2149231},
    URL = {https://doi.org/10.1080/00295639.2022.2149231},
 eprint = {https://doi.org/10.1080/00295639.2022.2149231}
}
```

### Fluid-structure interaction

This paper documents the development of the acoustic FSI capabilities and its verification and experimental validation.

```
@article{dhulipala2022acousticfsi,
  title = {Development, verification, and validation of comprehensive acoustic fluid-structure interaction capabilities in an open-source computational platform},
 author = {Dhulipala, Somayajulu L. N. and Bolisetti, Chandrakanth and Munday, Lynn B. and Hoffman, William M. and Yu, Ching-Ching and Mir, Faizan U. H. and Kong, Fande and Lindsay, Alexander D. and Whittaker, Andrew S.},
journal = {Earthquake Engineering and Structural Dynamics},
   year = {2022}
  month = {May},
  pages = {1--33},
 doi    = {10.1002/eqe.3659},
 url    = {https://doi.org/10.1002/eqe.3659}
}
```

### Fluid Properties Module

This paper documents all the fluid properties implemented in the module and their capabilities. If your input includes a `[FluidProperties]`
section, it is relying on this module.

```
@article{GIUDICELLI2025109407,
title = {The MOOSE fluid properties module},
journal = {Computer Physics Communications},
volume = {307},
pages = {109407},
year = {2025},
issn = {0010-4655},
doi = {https://doi.org/10.1016/j.cpc.2024.109407},
url = {https://www.sciencedirect.com/science/article/pii/S0010465524003308},
author = {Guillaume Giudicelli and Christopher Green and Joshua Hansel and David Andrs and April Novak and Sebastian Schunert and Benjamin Spaude and Steven Isaacs and Matthias Kunick and Robert Salko and Shane Henderson and Lise Charlot and Alexander Lindsay},
keywords = {Modeling & simulation, Fluid properties, MOOSE},
}
```
Available on OSTI at [https://www.osti.gov/pages/biblio/2476597] after October 2025.

### Electromagnetics Module

!include modules/electromagnetics/index.md start=em-citation-start end=em-citation-end

### Stochastic Tools Module

The following paper documents functionalities and demonstration of the MOOSE stochastic tools module.

```
@article{slaughter2023moose,
  title={MOOSE Stochastic Tools: A module for performing parallel, memory-efficient in situ stochastic simulations},
  author={Slaughter, Andrew E and Prince, Zachary M and German, Peter and Halvic, Ian and Jiang, Wen and Spencer, Benjamin W and Dhulipala, Somayajulu LN and Gaston, Derek R},
  journal={SoftwareX},
  volume={22},
  pages={101345},
  year={2023},
  publisher={Elsevier}
}
```

### Phase Field Module

```
@article{schwen2023phasefield,
  author = {D. Schwen and L.K. Aagesen and J.W. Peterson and M.R. Tonks}
  title = {Rapid multiphase-field model development using a modular free energy based approach with automatic differentiation in MOOSE/MARMOT},
  journal = {Computational Materials Science},
  volume = {132},
  pages = {36-45},
  year = {2017},
  doi = {https://doi.org/10.1016/j.commatsci.2017.02.017},
  url = {https://www.sciencedirect.com/science/article/pii/S0927025617300885},
}
```

### Optimization Module

```
@article{PRINCE2024101754,
author = {Zachary M. Prince and Lynn Munday and Dewen Yushu and Max Nezdyur and Murthy Guddati}
title = {MOOSE Optimization Module: Physics-constrained optimization},
journal = {SoftwareX},
volume = {26},
pages = {101754},
year = {2024},
doi = {https://doi.org/10.1016/j.softx.2024.101754},
url = {https://www.sciencedirect.com/science/article/pii/S2352711024001250},
}
```
