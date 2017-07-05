
#from setup.py
DISTRIBUTION_SOURCE=$(CROW_DIR)/src/distributions/distribution.cxx $(CROW_DIR)/src/utilities/MDreader.cxx $(CROW_DIR)/src/utilities/inverseDistanceWeigthing.cxx $(CROW_DIR)/src/utilities/microSphere.cxx $(CROW_DIR)/src/utilities/NDspline.cxx $(CROW_DIR)/src/utilities/ND_Interpolation_Functions.cxx  $(CROW_DIR)/src/distributions/distributionNDBase.cxx $(CROW_DIR)/src/distributions/distributionNDNormal.cxx $(CROW_DIR)/src/distributions/distributionFunctions.cxx $(CROW_DIR)/src/distributions/DistributionContainer.cxx $(CROW_DIR)/src/distributions/distribution_1D.cxx

#from setup.py
INTERPOLATION_SOURCE=$(CROW_DIR)/src/utilities/ND_Interpolation_Functions.cxx $(CROW_DIR)/src/utilities/NDspline.cxx $(CROW_DIR)/src/utilities/microSphere.cxx $(CROW_DIR)/src/utilities/inverseDistanceWeigthing.cxx $(CROW_DIR)/src/utilities/MDreader.cxx



$(CROW_DIR)/install/crow_modules/_distribution1Dpy2.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy2.so : setup_py2_intermediate

.INTERMEDIATE: setup_py2_intermediate

setup_py2_intermediate : $(CROW_DIR)/crow_modules/distribution1Dpy2.i $(CROW_DIR)/crow_modules/interpolationNDpy2.i $(DISTRIBUTION_SOURCE) $(INTERPOLATION_SOURCE)
	(cd $(CROW_DIR) && unset CXX CC && CFLAGS="$$CFLAGS $(COVERAGE_COMPILE_EXTRA)" && LDFLAGS="$$LDFLAGS $(COVERAGE_LINK_EXTRA)" && export CFLAGS LDFLAGS &&if test `uname` = Darwin; then MACOSX_DEPLOYMENT_TARGET=10.9; export MACOSX_DEPLOYMENT_TARGET; fi && . ./scripts/setup_raven_libs && python ./setup.py build_ext build install --install-platlib=./install)

$(CROW_DIR)/install/crow_modules/_distribution1Dpy3.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy3.so : setup_py3_intermediate

.INTERMEDIATE: setup_py3_intermediate

setup_py3_intermediate : $(CROW_DIR)/crow_modules/distribution1Dpy3.i $(CROW_DIR)/crow_modules/interpolationNDpy3.i $(DISTRIBUTION_SOURCE) $(INTERPOLATION_SOURCE)
	(cd $(CROW_DIR) && unset CXX CC && if test `uname` = Darwin; then MACOSX_DEPLOYMENT_TARGET=10.9; export MACOSX_DEPLOYMENT_TARGET; fi && . ./scripts/setup_raven_libs && python3 ./setup3.py build_ext build install --install-platlib=./install)
