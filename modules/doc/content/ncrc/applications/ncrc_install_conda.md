## Install Conda

!include getting_started/installation/install_miniconda.md

## Install NCRC Client

The NCRC client is available via INL's public Conda channel repository.

!alert note
The ncrc client must be installed and used while in the `(base)` Conda environment. The only exception to this, is when one performs an update (described later).

Add INL Conda Repository channel:

```bash
conda config --add channels https://conda.software.inl.gov/public
conda install ncrc
```
