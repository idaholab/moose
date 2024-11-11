
!versioner! code
conda activate base
conda create --yes --name moose moose-dev=__VERSIONER_CONDA_VERSION_MOOSE_DEV__ --channel https://conda.software.inl.gov/public
conda activate moose
!versioner-end!
