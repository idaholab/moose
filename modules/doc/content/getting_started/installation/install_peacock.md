
Peacock requires many third party libraries. The easiest way to get these libraries installed is to
use our `moose-peacock` package available from INL's public Conda channel.

```bash
conda config --add channels https://conda.software.inl.gov/public
mamba create -n peacock moose-peacock
```

!alert note title
It is safe to ignore a warning about our public channel already among your channel list.

!alert warning title=Separate Environments
It is important to create a new environment to contain the libraries necessary to run Peacock. Do
not install `moose-peacock` while inside the `moose` environment (or any environment with
`moose-libmesh` installed).

Activate the newly created `peacock` environment:

```bash
mamba activate peacock
```

!alert note
When ever you wish to run Peacock, it will be necessary to activate this environment.