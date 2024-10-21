Update Conda:

!versioner! code
conda activate base
conda env remove -n moose
conda create -n moose moose-dev=__VERSIONER_CONDA_VERSION_MOOSE_DEV__
conda activate moose
!versioner-end!

!alert note title=Always Update MOOSE and the Conda packages together
There is a tight dependency between libraries being provided by Conda, and what libraries MOOSE
depends on. Therefore, when you update one you should always update the other.
