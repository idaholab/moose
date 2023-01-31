!include getting_started/installation/install_miniconda.md

Before we continue, we first need to initialize mamba. Execute the following command and +restart your terminal session+.

```bash
$ ~> mamba init
```

Upon restarting your terminal, you should see your prompt prefixed with (base). This indicates you are in the base environment, and Conda is ready for operation:

```bash
$ (base) ~>
```

## Install NCRC Client

The NCRC client is available via INL's public Conda channel repository.

!alert note
The ncrc client must be installed and used while in the `(base)` Conda environment. The only exception to this, is when one performs an update (described later).

Add INL Conda Repository channel:

```bash
$ (base) ~> conda config --add channels https://conda.software.inl.gov/public
$ (base) ~> conda install ncrc
```
