
## Python (possibly optional)

Building MOOSE requires having Python headers (python-devel) and the Python package: `packaging`
available. If you know you have this requirement satisfied, and you do not plan on also using
Peacock (next section) you can skip this section.

These requirements are easily installed via Conda.

!include getting_started/installation/install_miniconda.md

Next, install the necessary `packaging` module:

```bash
mamba install packaging
```

## Peacock (optional)

!include installation/install_peacock.md
