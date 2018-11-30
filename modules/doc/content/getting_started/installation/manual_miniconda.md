## Miniconda

Peacock (an optional MOOSE GUI frontend) uses many libraries. The easiest way to obtain these libraries, is to install miniconda, along with several miniconda/pip packages.

!package! code max-height=500
cd $STACK_SRC
curl -L -O https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh
sh Miniconda2-latest-Linux-x86_64.sh -b -p $PACKAGES_DIR/miniconda

PATH=$PACKAGES_DIR/miniconda/bin:$PATH conda config --set ssl_verify false
PATH=$PACKAGES_DIR/miniconda/bin:$PATH conda install -c idaholab python=__CONDA_PYTHON__ coverage \
reportlab \
mako \
numpy \
scipy \
scikit-learn \
h5py \
hdf5 \
scikit-image \
requests \
vtk=__CONDA_VTK__ \
pyyaml \
matplotlib \
pip \
lxml \
pyflakes \
pandas \
conda-build \
mock \
yaml \
pyqt \
swig --yes
!package-end!


!alert note
Peacock (as well as the TestHarness sytem in MOOSE), does not work with Python3. Please chose Miniconda2 for Python 2.7 instead.


Next, we need to use `pip` to install additional libraries not supplied by conda:

!package! code
PATH=$PACKAGES_DIR/miniconda/bin:$PATH pip install --no-cache-dir pybtex livereload==__PIP_LIVERELOAD__ daemonlite pylint==__PIP_PYLINT__ lxml pylatexenc anytree
!package-end!
