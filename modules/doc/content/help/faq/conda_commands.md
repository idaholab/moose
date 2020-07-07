# Conda Command Reference

A list of common Conda commands, and when you might want to use them.


## activate id=condaactivate

*Useful when you want python 2.7 in one environment, and python 3.8 in another.*

```language=bash
conda activate <environment name>

# eg:
conda activate moose
```

Used to enable or 'activate' an environment separate from a previous environment. By activating an environment, you are thus deactivating a previous one. If you find yourself installing *and* uninstalling Conda packages frequently, you will benefit from using multiple environments instead (activating them as needed).

## create id=condacreate

*Useful when you need different libraries available where those -different libraries- may conflict with eachother.*

```language=bash
# A new bare environment:
conda create --name <environment name>

# eg:
conda create --name moose



# A new environment with specified packages installed:
conda create --name <environment name> <space separated list of packages>

# eg:
conda create --name moose moose-libmesh moose-tools
```

Used to create new environments. You can create as many environments as your hard drive space allows for. Once created, you can then [activate](conda_commands.md#condaactivate) said environment.

!alert note
When specifying packages for installation, `conda create` supports version control. See [install](conda_commands.md#condainstall) below

## deactivate id=condadeactivate

```language=bash
conda deactivate
```

Used to return to the base environment.

## install id=condainstall

*Useful when installing packages in addition to those when you first created the environment.*

```language=bash
conda install space-separated-list
```

Used to install the latest version of those packages into which ever environment is currently active. You can also specify versions, and even specific builds of that version:

```language=bash
conda install moose-petsc=3.12.5=build_1
```

## remove id=condaremove

*Useful when uninstalling the entire moose environment.*

```language=bash
conda remove --name <environment name> space-separated-list
```

Used to remove the space separated list of packages from the specified environment, while not actually having that environment activated. The more useful function of this command, is to specify the `--all` argument. Which will remove all packages from the specified environment, and then the actual environment itself:

```language=bash
conda remove --name <environment name> --all
```

