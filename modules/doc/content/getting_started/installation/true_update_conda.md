Update Conda:

!package! code
mamba activate base
mamba env remove -n moose
mamba create -n moose moose-dev=__MOOSE_DEV__
mamba activate moose
!package-end!

!alert note title=Always Update MOOSE and the Conda/Mamba packages together
There is a tight dependency between libraries being provided by Conda, and what libraries MOOSE
depends on. Therefore, when you update one you should always update the other.
