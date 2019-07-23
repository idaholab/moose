# Citing MOOSE

## Framework

If you use MOOSE for your publication, please cite the following:

```
@Misc{moose-web-page,
  author = {Brian Alger and David Andr{\v{s}} and Robert W. Carlsen and Derek R.
            Gaston and Fande Kong and Alexander D. Lindsay and Jason M. Miller and
            Cody J. Permann and John W. Peterson and Andrew E. Slaughter and Roy Stogner},
  title = {{MOOSE} {W}eb page},
  url = {https://mooseframework.org},
  howpublished = {\url{https://mooseframework.org}},
  year = {2019}
}

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

If you use the multi-surface plasticity capability, `ComputeMultiPlasticityStress`, of the Tensor Mechanics module, or if you just want to demonstrate MOOSE's advanced plasticity features, please cite:

```
@article{article,
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
