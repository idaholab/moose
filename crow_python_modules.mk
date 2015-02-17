
DISTRIBUTION_COMPILE_COMMAND=$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) -I$(CROW_LIB_INCLUDE_DIR) -I$(CROW_DIR)/include/distributions/ -I$(CROW_DIR)/include/utilities/  -MMD -MF $@.d -MT $@ -c $< -o $@

$(CROW_DIR)/src/distributions/DistributionContainer.$(obj-suffix): $(CROW_DIR)/src/distributions/DistributionContainer.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/distribution_1D.$(obj-suffix): $(CROW_DIR)/src/distributions/distribution_1D.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/distributionFunctions.$(obj-suffix): $(CROW_DIR)/src/distributions/distributionFunctions.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/distributions/distribution_base_ND.$(obj-suffix): $(CROW_DIR)/src/distributions/distribution_base_ND.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix): $(CROW_DIR)/src/utilities/ND_Interpolation_Functions.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(CROW_DIR)/install/crow_modules/_distribution1Dpy2.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy2.so : $(CROW_DIR)/crow_modules/distribution1Dpy2.i $(CROW_DIR)/crow_modules/interpolationNDpy2.i
	(cd $(CROW_DIR) && python $(CROW_DIR)/setup.py build_ext build install --install-platlib=$(CROW_DIR)/install)
