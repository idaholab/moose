
DISTRIBUTION_COMPILE_COMMAND=$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) -I$(CROW_LIB_INCLUDE_DIR) -I$(CROW_DIR)/include/distributions/ -I$(CROW_DIR)/include/utilities/  -MMD -MF $@.d -MT $@ -c $< -o $@

#from setup.py
DISTRIBUTION_SOURCE=$(CROW_DIR)/src/distributions/distribution.C $(CROW_DIR)/src/utilities/MDreader.C $(CROW_DIR)/src/utilities/inverseDistanceWeigthing.C $(CROW_DIR)/src/utilities/microSphere.C $(CROW_DIR)/src/utilities/NDspline.C $(CROW_DIR)/src/utilities/ND_Interpolation_Functions.C  $(CROW_DIR)/src/distributions/distributionNDBase.C $(CROW_DIR)/src/distributions/distributionNDNormal.C $(CROW_DIR)/src/distributions/distributionFunctions.C $(CROW_DIR)/src/distributions/DistributionContainer.C $(CROW_DIR)/src/distributions/distribution_1D.C

#from setup.py
INTERPOLATION_SOURCE=$(CROW_DIR)/src/utilities/ND_Interpolation_Functions.C $(CROW_DIR)/src/utilities/NDspline.C $(CROW_DIR)/src/utilities/microSphere.C $(CROW_DIR)/src/utilities/inverseDistanceWeigthing.C $(CROW_DIR)/src/utilities/MDreader.C

$(CROW_DIR)/src/distributions/DistributionContainer.$(obj-suffix): $(CROW_DIR)/src/distributions/DistributionContainer.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/distribution_1D.$(obj-suffix): $(CROW_DIR)/src/distributions/distribution_1D.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/distributionFunctions.$(obj-suffix): $(CROW_DIR)/src/distributions/distributionFunctions.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/distributionNDBase.$(obj-suffix): $(CROW_DIR)/src/distributions/distributionNDBase.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/distributionNDNormal.$(obj-suffix): $(CROW_DIR)/src/distributions/distributionNDNormal.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix): $(CROW_DIR)/src/utilities/ND_Interpolation_Functions.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/randomClass.$(obj-suffix): $(CROW_DIR)/src/distributions/randomClass.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/distributionNDCartesianSpline.$(obj-suffix): $(CROW_DIR)/src/distributions/distributionNDCartesianSpline.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/install/crow_modules/_distribution1Dpy2.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy2.so : $(CROW_DIR)/crow_modules/distribution1Dpy2.i $(CROW_DIR)/crow_modules/interpolationNDpy2.i $(DISTRIBUTION_SOURCE) $(INTERPOLATION_SOURCE)
	(cd $(CROW_DIR) && unset CXX CC && CFLAGS="$$CFLAGS $(COVERAGE_COMPILE_EXTRA)" && LDFLAGS="$$LDFLAGS $(COVERAGE_LINK_EXTRA)" && export CFLAGS LDFLAGS &&if test `uname` == Darwin; then MACOSX_DEPLOYMENT_TARGET=10.9; export MACOSX_DEPLOYMENT_TARGET; fi && python $(CROW_DIR)/setup.py build_ext build install --install-platlib=$(CROW_DIR)/install)

$(CROW_DIR)/install/crow_modules/_distribution1Dpy3.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy3.so : $(CROW_DIR)/crow_modules/distribution1Dpy3.i $(CROW_DIR)/crow_modules/interpolationNDpy3.i $(DISTRIBUTION_SOURCE) $(INTERPOLATION_SOURCE)
	(cd $(CROW_DIR) && unset CXX CC && if test `uname` == Darwin; then MACOSX_DEPLOYMENT_TARGET=10.9; export MACOSX_DEPLOYMENT_TARGET; fi && python3 $(CROW_DIR)/setup3.py build_ext build install --install-platlib=$(CROW_DIR)/install)
