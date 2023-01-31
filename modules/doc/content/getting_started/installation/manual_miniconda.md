## Miniconda

Peacock (an optional MOOSE GUI frontend) uses many libraries. The easiest way to obtain these libraries, is to install miniconda, along with several miniconda/pip packages.

!package! code max-height=500
cd $STACK_SRC
curl -L -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
sh Miniconda3-latest-Linux-x86_64.sh -b -p $PACKAGES_DIR/miniconda

PATH=$PACKAGES_DIR/miniconda/bin:$PATH conda install coverage \
reportlab \
mako \
numpy \
scipy \
scikit-learn \
h5py \
hdf5 \
scikit-image \
requests \
vtk \
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

Next, we need to use `pip` to install additional libraries not supplied by conda:

!package! code
PATH=$PACKAGES_DIR/miniconda/bin:$PATH pip install --no-cache-dir pybtex livereload daemonlite pylint lxml pylatexenc anytree
!package-end!

