## Updating

Due to how Conda manages dependencies, we typically suggest that you recreate the environment instead of update it.

If you have already followed the instructions above to create the environment, you should then run:

!versioner! code
conda env remove -n moose
conda create -n moose moose-dev=__VERSIONER_CONDA_VERSION_MOOSE_DEV__
!versioner-end!

This will remove the previous environment and create a new environment with the current version.

After running the above, then run:

```bash
conda activate moose
```

as usual to utilize the environment.

## Uninstall

To uninstall the development environment, run the following:

```bash
conda activate base
conda env remove -n moose
```
