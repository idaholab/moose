# The chemical database

Notation and definitions are described in [geochemistry_nomenclature.md].

This page describes the GWB format, using a database file downloaded from the geochemist workbench site.  See the "Thermo Datasets" chapter of [!cite](gwb_reference) and [https://academy.gwb.com](https://academy.gwb.com)

A database in GWB format can be converted to a MOOSE-formatted database using the supplied python utility

```bash
geochemistry/python/database_converter.py -i gwb_database --format gwb -o moose_database.json
```

The following details the GWB database section-by-section.

## Descriptive header

The database file begins with a descriptive header:

```
dataset of thermodynamic data for gwb programs
dataset format: jan19
activity model: debye-huckel
fugacity model: tsonopoulos
```

!alert note
Only the [Debye-Huckel activity model](activity_coefficients.md) model is currently supported.
Only the [Spycher-Reed fugacity model](fugacity.md) model is currently supported.

## Temperature definition

All equilibrium coefficients, activity coefficients, etc, are evaluated at a set of temperatures.  These are assumed to be in $^{\circ}$C, and defined by the line(s)

```
* temperatures
         0.0000     25.0000     60.0000    100.0000
       150.0000    200.0000    250.0000    300.0000
```

If a numerical simulation is performed at a temperature that is not one of these 8 values, a fourth-order polynomial fit to the data is used.

## Steam saturation curve

The pressures --- in bars (1 bar $= 10^{5}\,$Pa) --- along the steam saturation curve, at the temperatures defined above are given in the form

```
* pressures
         1.0134      1.0134      1.0134      1.0134
         4.7600     15.5490     39.7760     85.9270
```

## Debye-Huckel coefficients

The [Debye-Huckel coefficients](activity_coefficients.md) are given next.  These are evaluated at the temperatures defined above

```
* debye huckel a (adh)
          .4913       .5092       .5450       .5998
          .6898       .8099       .9785      1.2555
* debye huckel b (bdh)
          .3247       .3283       .3343       .3422
          .3533       .3655       .3792       .3965
* bdot
          .0174       .0410       .0440       .0460
          .0470       .0470       .0340      0.0000
```

The units are

- $A$ has units mol$^{-1/2}$.kg$^{1/2}$;
- $B$ has units mol$^{-1/2}$.kg$^{1/2}$.$\mathring{A}^{-1}$;
- $\dot{B}$ has units mol$^{-1}$.kg.

So, for instance, at 25$^{\circ}$C

- $A=0.5092\,$mol$^{-1/2}$.kg$^{1/2}$,
- $B=0.3283\,$mol$^{-1/2}$.kg$^{1/2}$.$\mathring{A}^{-1}$ and
- $\dot{B}=0.0410\,$mol$^{-1}$.kg.


## Neutral species activity coefficients

The temperature dependence of the parameters $a$, $b$, $c$ and $d$ that define the activity coefficients for the neutral species:

```
* c co2 1
          .1224       .1127      .09341      .08018
         .08427      .09892       .1371       .1967
* c co2 2
       -.004679     -.01049      -.0036    -.001503
        -.01184      -.0104    -.007086     -.01809
* c co2 3
      -.0004114     .001545    9.609e-5    .0005009
        .003118     .001386    -.002887    -.002497
* c co2 4
         0.0000      0.0000      0.0000      0.0000
         0.0000      0.0000      0.0000      0.0000
```

The formulae for neutral species is given in the [activity coefficients](activity_coefficients.md) page, and the special keyword $\mathring{a}=-0.5$ acts as a flag to the solver to use this instead of the Debye-Huckel formula (for instance, see the basis species B(OH)3, below).  See Section 3.1.4 of [!cite](gwb_reference).

## Water activity

The [activity of water](activity_coefficients.md) is defined by four coefficients, $\tilde{a}$, $\tilde{b}$, $\tilde{d}$ and $\tilde{d}$, whose temperature dependence is given by

```
* c h2o 1
       500.0000     1.45397    500.0000      1.5551
         1.6225    500.0000    500.0000    500.0000
* c h2o 2
       500.0000     .022357    500.0000     .036478
        .045891    500.0000    500.0000    500.0000
* c h2o 3
       500.0000    .0093804    500.0000    .0064366
       .0045221    500.0000    500.0000    500.0000
* c h2o 4
       500.0000   -.0005362    500.0000   -.0007132
      -.0008312    500.0000    500.0000    500.0000
```

Note that only their values at 25$^{\circ}$C, 100$^{\circ}$C and 200$^{\circ}$C are given: the values of 500.0000 indicates "no value".

## Element definitions

The names, chemical abbreviations and mole weights are provided by the following lines:

```
   46 elements

Silver          (Ag)          mole wt.=  107.8680 g
Aluminum        (Al)          mole wt.=   26.9815 g
Americium       (Am)          mole wt.=  241.0600 g
...
-end-
```

## Basis species

A list of the basis species and their properties are given by:

```
   47 basis species

H2O
     charge=  0      ion size=  0.0 A      mole wt.=   18.0152 g
     2 elements in species
        2.000 H               1.000 O       

Ag+
     charge=  1      ion size=  2.5 A      mole wt.=  107.8680 g
     1 elements in species
        1.000 Ag      
...
B(OH)3
     charge=  0      ion size=  -.5 A      mole wt.=   61.8329 g
     3 elements in species
        1.000 B               3.000 H               3.000 O       
...
-end-
```

In these entries:

- The first line defines the name (H2O, Ag+, etc)
- The second line provides the charge, $z$, and ion size, $\mathring{a}$ (both used in computing the [activity](activity_coefficients.md)) and the molecular weight.  For neutral species, the [activity](activity_coefficients.md) is calculated (see Section 3.1.4 of [!cite](gwb_reference)):

  - If `ion size=0` and the species is `H2O`, the formula for the activity of water is used
  - If `ion size=0` and the species is not `H2O`, the activity is set to 1.
  - If `ion size=-0.5A` the activity is calculated using the CO2 coefficients
  - If `ion size<=-1A` the formula $a=\log_{10}\dot{B}I$ is used.
- The remaining data provides the elemental decomposition.

## Redox pairs

The database also contains redox pair information in the form:

```
   48 redox couples

(O-phth)--
     charge= -2      ion size=  4.0 A      mole wt.=  164.1172 g
     4 species in reaction
       -5.000 H2O             8.000 HCO3-           6.000 H+      
       -7.500 O2(aq)  
       594.3211    542.8292    482.3612    425.9738
       368.7004    321.8658    281.8216    246.4849

Am++++
     charge=  4      ion size= 11.0 A      mole wt.=  241.0600 g
     4 species in reaction
        -.500 H2O             1.000 H+              1.000 Am+++   
         .250 O2(aq)  
        18.7967     18.0815     17.2698     16.5278
        15.8024     15.2312     14.7898     14.4250
...
-end-
```

In these entries:

- The first line defines the name ((O-phth)--, Am++++, etc)
- The second line provides the charge, $z$, and ion size, $\mathring{a}$ and the molecular weight.  The former two are used to compute the [activity](activity_coefficients.md) for ["decoupled" redox pairs](basis.md).
- The remaining data provides the equilibrium reaction (in terms of the basis species, or any redox species that have been defined so far), along with its temperature-dependent equilibrium constant, which are used when the redox pair is ["coupled"](basis.md), for two purposes: (a) to eliminate the alternative oxidataion state ((O-phth)--, Am++++, etc) from all reactions in favour of the basis species; (b) to form another secondary-species reaction.

## Aqueous species

Much of the database concerns equilibrium reactions to produce secondary species:

```
   551 aqueous species

(NpO2)2(OH)2++
     charge=  2      ion size=  5.0 A      mole wt.=  572.1086 g
     3 species in reaction
       -2.000 H+              2.000 H2O             2.000 NpO2++  
         7.0580      6.2979      5.5317      4.9568
         4.5346      4.3362      4.2830      4.3258

(NpO2)3(OH)5+
     charge=  1      ion size=  4.0 A      mole wt.=  892.1775 g
     3 species in reaction
       -5.000 H+              5.000 H2O             3.000 NpO2++  
        19.2691     17.3865     15.4529     13.9264
        12.6876     11.9568     11.5491     11.3537
...
-end-
```

In these entries:

- The first line defines the name ((NpO2)2(OH)2$++$, etc)
- The second line provides the charge, $z$, and ion size, $\mathring{a}$ and the molecular weight.  The first two are used to compute the activity coefficient for the species.
- The remaining data provides the equilibrium reaction (in terms of basis species and redox couples) along with its temperature-dependent equilibrium constant.

## Free electron

The data base contains:

```
   1 free electron

e-
     charge= -1      ion size=  0.0 A      mole wt.=    0.0000 g
     3 species in reaction
         .500 H2O             -.250 O2(g)          -1.000 H+      
       22.76135     20.7757   18.513025     16.4658
      14.473225    12.92125    11.68165    10.67105

-end-
```

## Minerals

Mineralisation reactions are defined in ths following block:

```
   624 minerals

(BaO)2^(SiO2)3(c)                  type=
     formula=
     mole vol.=  122.8000 cc      mole wt.=  486.9117 g
     4 species in reaction
       -4.000 H+              2.000 Ba++            3.000 SiO2(aq)
        2.000 H2O     
        24.8965     23.3104     21.2301     19.2286
        17.2446     15.6907     14.3849     13.0572

...
Acanthite                          type= sulfide
     formula= Ag2S
     mole vol.=   34.2000 cc      mole wt.=  247.7960 g
     3 species in reaction
        2.000 Ag+            -1.000 H+              1.000 HS-     
       -39.7042    -36.0478    -31.9375    -28.2491
       -24.6984    -22.0245    -20.0510    -18.7392
...
-end-
```

In these entries:

- The first line defines the name ((BaO)2^(SiO2)3(c), Acanthite, etc)
- The "type" and "formula" are optional, and are not used in the `geochemistry` module
- The mole volume and mole weight are quantified (the activity for each mineral is unity)
- The remaining data provides the equilibrium reaction (in terms of basis species and redox couples) along with its temperature-dependent equilibrium constant.

## Gases

The thermodynamics of gases and reactions involving them are included in the database as:

```

   10 gases

CH4(g)
     mole wt.=   16.0426 g
     chi=       -537.779     1.54946 -.000927827     1.20861  -.00370814  3.33804e-6
     Pcrit=     46.0 bar      Tcrit=     190.4 K      omega=        .011
     1 species in reaction
        1.000 CH4(aq)
        -2.6487     -2.8202     -2.9329     -2.9446
        -2.9163     -2.7253     -2.4643     -2.1569
...
-end-
```

In these blocks:

- The first line defines the name (CH4(g), etc)
- The "chi" line defines the [Spycher-Reed coefficients](fugacity.md)
- The "Pcrit" line is used to evaluate Tsonopoulos and Peng-Robinson pressure models, and are not used in the `geochemistry` module
- The remaining data define the equilibrium reaction and its constant.

!bibtex bibliography
