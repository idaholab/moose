
#from setup.py
DISTRIBUTION_SOURCE=$(CROW_DIR)/src/distributions/distribution.C $(CROW_DIR)/src/utilities/MDreader.C $(CROW_DIR)/src/utilities/inverseDistanceWeigthing.C $(CROW_DIR)/src/utilities/microSphere.C $(CROW_DIR)/src/utilities/NDspline.C $(CROW_DIR)/src/utilities/ND_Interpolation_Functions.C  $(CROW_DIR)/src/distributions/distributionNDBase.C $(CROW_DIR)/src/distributions/distributionNDNormal.C $(CROW_DIR)/src/distributions/distributionFunctions.C $(CROW_DIR)/src/distributions/DistributionContainer.C $(CROW_DIR)/src/distributions/distribution_1D.C

#from setup.py
INTERPOLATION_SOURCE=$(CROW_DIR)/src/utilities/ND_Interpolation_Functions.C $(CROW_DIR)/src/utilities/NDspline.C $(CROW_DIR)/src/utilities/microSphere.C $(CROW_DIR)/src/utilities/inverseDistanceWeigthing.C $(CROW_DIR)/src/utilities/MDreader.C



$(CROW_DIR)/install/crow_modules/_distribution1Dpy2.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy2.so : setup_py2_intermediate

.INTERMEDIATE: setup_py2_intermediate

setup_py2_intermediate : $(CROW_DIR)/crow_modules/distribution1Dpy2.i $(CROW_DIR)/crow_modules/interpolationNDpy2.i $(DISTRIBUTION_SOURCE) $(INTERPOLATION_SOURCE)
	(cd $(CROW_DIR) && unset CXX CC && CFLAGS="$$CFLAGS $(COVERAGE_COMPILE_EXTRA)" && LDFLAGS="$$LDFLAGS $(COVERAGE_LINK_EXTRA)" && export CFLAGS LDFLAGS &&if test `uname` = Darwin; then MACOSX_DEPLOYMENT_TARGET=10.9; export MACOSX_DEPLOYMENT_TARGET; fi && . $(CROW_DIR)/scripts/setup_raven_libs && python $(CROW_DIR)/setup.py build_ext build install --install-platlib=$(CROW_DIR)/install)

$(CROW_DIR)/install/crow_modules/_distribution1Dpy3.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy3.so : setup_py3_intermediate

.INTERMEDIATE: setup_py3_intermediate

setup_py3_intermediate : $(CROW_DIR)/crow_modules/distribution1Dpy3.i $(CROW_DIR)/crow_modules/interpolationNDpy3.i $(DISTRIBUTION_SOURCE) $(INTERPOLATION_SOURCE)
	(cd $(CROW_DIR) && unset CXX CC && if test `uname` = Darwin; then MACOSX_DEPLOYMENT_TARGET=10.9; export MACOSX_DEPLOYMENT_TARGET; fi && . $(CROW_DIR)/scripts/setup_raven_libs && python3 $(CROW_DIR)/setup3.py build_ext build install --install-platlib=$(CROW_DIR)/install)
