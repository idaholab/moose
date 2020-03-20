# The chemical database

Notation and definitions are described in [geochemistry_nomenclature.md].

The `geochemistry` module works with chemical databases in a simple [JSON](https://www.json.org) format,
making it easy to parse in a variety of applications or languages.

## Python database converter

A python utility The module contains a python utility that translates between the MOOSE format and other formats such as EQ3/6 or [GWB](gwb_database.md). A database in either of these formats can be translated to a MOOSE JSON database using

```
geochemistry/python/database_converter.py -i gwb_database --format gwb -o moose_database.json
```

for example. Using the supplied python conversion utility ensures that the database produced adheres to the required
format.

A full list of the functionality can be obtained using the `--help` flag

```
geochemistry/python/database_converter.py --help
usage: database_converter.py [-h] -i INPUT --format {gwb,eq36} [-o OUTPUT]

Utility to read data from thermodynamic database and write to MOOSE
thermodynamic database (in JSON format)

optional arguments:
  -h, --help            show this help message and exit
  -i INPUT, --input INPUT
                        The input database
  --format {gwb,eq36}   The database format
  -o OUTPUT, --output OUTPUT
                        The output filename. Default: moose_thermo.json
```

## Database format

The MOOSE thermodynamic database is a [JSON](https://www.json.org) file with key-value pairs for the various species and
datatypes. The specific structure of the MOOSE database is outlined below.

### Header information

The required `Header` field contains several optional provenance fields (such as the original database format and filename), as well as some required fields (such as temperature points for equilibrium constant evaluation, activity coefficients, etc).

An example of the optional header information is shown below:

```
"Header": {
  "title": "MOOSE thermodynamic database",
  "original": "{$MOOSE_DIR}/projects/moose/modules/geochemistry/python/original_dbs/gwb_llnl.dat",
  "date": "22:00 15-03-2020",
  "original format": "gwb"
}
```

The `Header` field must include information about the temperatures and pressures that the equilibrium constant values or activity coefficients are defined at:

```
"Header": {
  "temperatures": [
    "0.0000",
    "25.0000",
    "60.0000",
    "100.0000",
    "150.0000",
    "200.0000",
    "250.0000",
    "300.0000"
  ],
  "pressures": [
    "1.0134",
    "1.0134",
    "1.0134",
    "1.0134",
    "4.7600",
    "15.5490",
    "39.7760",
    "85.9270"
  ]
}
```

The temperatures are in $^{\circ}$C, and the pressures --- in bars (1 bar $= 10^{5}\,$Pa) --- lie along the steam saturation curve at the temperatures defined above.

The [activity](activity_coefficients.md) and [fugactity](fugacity.md) models must be defined in the `Header`, along with the appropriate coefficients (at the temperatures specified above):

```
"Header": {
  "activity model": "debye-huckel",
  "fugacity model": "tsonopoulos",
  "adh": [
    ".4913",
    ".5092",
    ".5450",
    ".5998",
    ".6898",
    ".8099",
    ".9785",
    "1.2555"
  ],
  "bdh": [
    ".3247",
    ".3283",
    ".3343",
    ".3422",
    ".3533",
    ".3655",
    ".3792",
    ".3965"
  ],
  "bdot": [
    ".0174",
    ".0410",
    ".0440",
    ".0460",
    ".0470",
    ".0470",
    ".0340",
    "0.0000"
  ]
}
```

In this case, the [Debye-Huckel](activity_coefficients.md) activity model is specified, along with the Debye-Huckel
parameters $A$ (mol$^{-1/2}$.kg$^{1/2}$), $B$ (mol$^{-1/2}$.kg$^{1/2}$.$\mathring{A}^{-1}$), and $\dot{B}$ (mol$^{-1}$.kg).

So, for instance, at 25$^{\circ}$C

- $A=0.5092\,$mol$^{-1/2}$.kg$^{1/2}$,
- $B=0.3283\,$mol$^{-1/2}$.kg$^{1/2}$.$\mathring{A}^{-1}$ and
- $\dot{B}=0.0410\,$mol$^{-1}$.kg.


The `Header` field also contains activity coefficients for neutral species, the formulae for which is given in the [activity coefficients](activity_coefficients.md) description.  Each neutral species has parameters $a$, $b$, $c$ and $d$ that define the activity coefficients for that species, eg:

```
"Header": {
  "neutral species": {
    "h2o": {
      "a": [
        "500.0000",
        "1.45397",
        "500.0000",
        "1.5551",
        "1.6225",
        "500.0000",
        "500.0000",
        "500.0000"
      ],
      "b": [
        "500.0000",
        ".022357",
        "500.0000",
        ".036478",
        ".045891",
        "500.0000",
        "500.0000",
        "500.0000"
      ],
      "c": [
        "500.0000",
        ".0093804",
        "500.0000",
        ".0064366",
        ".0045221",
        "500.0000",
        "500.0000",
        "500.0000"
      ],
      "d": [
        "500.0000",
        "-.0005362",
        "500.0000",
        "-.0007132",
        "-.0008312",
        "500.0000",
        "500.0000",
        "500.0000"
      ]
    }
  }
}
```

Note that the special keyword $\mathring{a}=-0.5$ acts as a flag to the solver to use this activity model instead of the Debye-Huckel formula.

!alert note
A value of `500` indicates "no value"

### Elements

Molecular weights (in g) of all elements present in the database are provided in the `elements` field.

```
"elements": {
  "Ag": {
    "name": "Silver",
    "molecular weight": "107.8680"
  },
  "Al": {
    "name": "Aluminum",
    "molecular weight": "26.9815"
  },
}
```

### Basis species

The `basis species` field contains all information about the basis species, for example

```
"basis species": {
  "H2O": {
    "elements": {
      "H": "2.000",
      "O": "1.000"
    },
    "charge": "0",
    "radius": "0.0",
    "molecular weight": "18.0152"
  },
  "Ag+": {
    "elements": {
      "Ag": "1.000"
    },
    "charge": "1",
    "radius": "2.5",
    "molecular weight": "107.8680"
  }
}
```

Each basis species has a number of required fields:

- `elements`: the elemental decomposition in an element-weight pair (for example, the basis species H2O has 2 hydrogen (H) molecules and 1 oxygen (O) molecule)
- `charge`: the electrical charge of the basis species
- `radius`: the ionic radius (A) of the basis species
- `molecular weight`: the molecular weight (g)

### Aqueous secondary species

The `secondary species` field contains all information about the aqueous equilibrium species

```
"basis species": {
  "CaCO3": {
    "species": {
      "Ca++": "1.000",
      "HCO3-": "1.000",
      "H+": "-1.000"
    },
    "charge": "0",
    "radius": "4.0",
    "molecular weight": "100.0892",
    "logk": [
      "7.5520",
      "7.1280",
      "6.7340",
      "6.4350",
      "6.1810",
      "5.9320",
      "5.5640",
      "4.7890"
    ]
  }
}
```

Each secondary species has a number of required fields:

- `species`: the basis species decomposition in a basis species-stoichiometry pair. In this example, `CaCO3` is  described by the reaction `CaCO3` $\rightleftharpoons$ `Ca++ + HCO3- - H+`
- `charge`: the electrical charge of the secondary species
- `radius`: the ionic radius (A) of the secondary species
- `molecular weight`: the molecular weight (g)
- `logk`: the equilibrium constant values at the temperature points specified in the `Header`

### Redox couples

The `redox couples` field contains all information about the redox pairs

```
"redox couples": {
  "(O-phth)--": {
    "species": {
      "H2O": "-5.000",
      "HCO3-": "8.000",
      "H+": "6.000",
      "O2(aq)": "-7.500"
    },
    "charge": "-2",
    "radius": "4.0",
    "molecular weight": "164.1172",
    "logk": [
      "594.3211",
      "542.8292",
      "482.3612",
      "425.9738",
      "368.7004",
      "321.8658",
      "281.8216",
      "246.4849"
    ]
  }
}
```

Each redox couple has a number of required fields:

- `species`: the basis species decomposition in a basis species-stoichiometry pair. In this example, Phenolphthalein `(O-phth)--)` is described by the redox reaction `(O-phth)-- + 5H2O + 7.5O2(aq)` $\rightleftharpoons$ `8HCO3- + 6H+`
- `charge`: the electrical charge of the redox species
- `radius`: the ionic radius (A) of the redox species
- `molecular weight`: the molecular weight (g)
- `logk`: the equilibrium constant values at the temperature points specified in the `Header`

### Minerals

The minerals are defined in the `mineral species` field

```
"mineral species": {
  "Acanthite": {
    "species": {
      "Ag+": "2.000",
      "H+": "-1.000",
      "HS-": "1.000"
    },
    "molar volume": "34.2000",
    "molecular weight": "247.7960",
    "logk": [
      "-39.7042",
      "-36.0478",
      "-31.9375",
      "-28.2491",
      "-24.6984",
      "-22.0245",
      "-20.0510",
      "-18.7392"
    ]
  }
}
```

Each mineral has a number of required fields:

- `species`: the basis species decomposition in a basis species-stoichiometry pair. In this example, `Acanthite` is described by the mineral reaction `Acanthite` $\rightleftharpoons$ `2Ag+ + HS1 - H+`
- `molecular weight`: the molecular weight (g)
- `molar volume`: the mineral volume (cc)
- `logk`: the equilibrium constant values at the temperature points specified in the `Header`

### Gases

The gases are defined in the `gas species` field

```
"gas species": {
  "CH4(g)": {
    "species": {
      "CH4(aq)": "1.000"
    },
    "molecular weight": "16.0426",
    "chi": [
      "-537.779",
      "1.54946",
      "-.000927827",
      "1.20861",
      "-.00370814",
      "3.33804e-6"
    ],
    "Pcrit": "46.0",
    "Tcrit": "190.4",
    "omega": ".011",
    "logk": [
      "-2.6487",
      "-2.8202",
      "-2.9329",
      "-2.9446",
      "-2.9163",
      "-2.7253",
      "-2.4643",
      "-2.1569"
    ]
  }
}
```

Each gas species has a number of required fields:

- `species`: the basis species decomposition in a basis species-stoichiometry pair. In this example, `CH4(g)` is described by the gas reaction `CH4(g)` $\rightleftharpoons$ `CH4(aq)`
- `molecular weight`: the molecular weight (g)
- `logk`: the equilibrium constant values at the temperature points specified in the `Header`

The gases can optionally include fugacity model parameters such as

- `chi`: the [Spycher-Reed](fugacity.md) fugacity coefficients
- `Pcrit, Tcrit` and `omega`: Tsonopoulos and Peng-Robinson model coefficients

### Surface species

Surface species in the database are listed in the `surface species` field.

```
"surface species": {
  ">(s)FeO-": {
    "species": {
      ">(s)FeOH": "1.000",
      "H+": "-1.000"
    },
    "charge": "-1",
    "molecular weight": "71.8464",
    "log K": "8.9300",
    "dlogK/dT": "0.0000"
  }
}
```

Each surface species has a number required fields:

- `species`: the decomposition in a basis species-stoichiometry pair as usual
- `charge`: the electrical charge of the secondary species
- `molecular weight`: the molecular weight (g)
- `log K` and `dlogK/dT`: the equilibrium constant and derivative wrt temperature

In contrast to the other types of reaction species, where the equilibrium constant is provided at each of
the temperature points listed in the `Header`, the surface species equilibrium constant is defined using
the value at the first temperature point, `log K` above, and the derivative wrt temperature
\begin{equation}
\log(K) = \mathtt{log K} + \mathtt{dlogK/dT} (T - T_0),
\end{equation}
where $\log$ is the natural logarithm, $T$ is the temperature where $\log(K)$ is to be calculated, and $T_0$ is the
first temperature specified in the `Header`.

For example, the equilibrium constant for the surface species `>(s)FeO-` is simply
\begin{equation}
\log(K) = 8.93,
\end{equation}
as the derivative wrt temperature is zero.

### Sorbing minerals

Sorbing minerals listed in the database provide sites where sorbing can occur for each mineral

```
"sorbing minerals": {
  "Goethite": {
    "sorbing sites": {
      ">(s)FeOH": ".0050",
      ">(w)FeOH": ".2000"
    },
    "surface area": "600.0000"
  }
}
```

Each sorbing mineral has two required fields:

- `sorbing sites`: list of sorbing sites as a basis species-site density pair
- `surface area`: the surface area available (m$^2$/g)

### Oxides

Oxides may be present in the database, for example

```
"oxides": {
  "Ag2O": {
    "species": {
      "H+": "-2.000",
      "Ag+": "2.000",
      "H2O": "1.000"
    },
    "molecular weight": "231.7350"
  }
}
```

Each oxide has two required fields:

- `species`: the basis species decomposition in a basis species-stoichiometry pair. In this example, silver oxide is described by the reaction `Ag2O + 2H+` $\rightleftharpoons$ `Ag+ + H2O`
- `molecular weight`: the molecular weight (g)


## Python utilities

As the MOOSE thermodynamic database is a standard JSON file, it can be easily parsed using python, allowing users
to readily access information from the database without needing a special reader. The `geochemistry` module provides some example python utilities in `python/dbutils.py` to illustrate how simple it is to examine the database using python.

For example, to find all secondary species that contain a particular basis species, the user can query the
database using the following python code

```python
import json
import dbutils

# Read the database
with open('geochemistry/database/moose_thermo.json', 'r') as dbfile:
    moosedb = json.load(dbfile)

# Find all aqueous equilibrium (secondary) species containing Ca++
dbutils.secondarySpeciesContainingBasis(moosedb, 'secondary species', ['Ca++'])
```

which returns

```
['Ca(H3SiO4)2',
 'Ca(O-phth)',
 'CaB(OH)4+',
 'CaCH3COO+',
 'CaCl+',
 'CaCO3',
 'CaF+',
 'CaH2SiO4',
 'CaH3SiO4+',
 'CaHCO3+',
 'CaHPO4',
 'CaNO3+',
 'CaOH+',
 'CaPO4-',
 'CaSO4']
```

Species information can be easily extracted

```python
import json
import dbutils

# Read the database
with open('geochemistry/database/moose_thermo.json', 'r') as dbfile:
    moosedb = json.load(dbfile)

# Find all aqueous equilibrium (secondary) species containing Ca++
dbutils.printSpeciesInfo(moosedb, 'Calcite')
```

giving

```
Calcite:
  type: mineral species
  species:  {'Ca++': '1.000', 'HCO3-': '1.000', 'H+': '-1.000'}
  molar volume:  36.9340
  molecular weight:  100.0892
  logk:  ['2.0683', '1.7130', '1.2133', '.6871', '.0762', '-.5349', '-1.2301', '-2.2107']
```

Additional utility functions to print reactions as formatted strings are also provided, see `python/dbutils.py`.
