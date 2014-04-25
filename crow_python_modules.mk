
RAVEN_PMODULES=$(RAVEN_DIR)/python_modules
# look for numpy include directory
#NUMPY_INCLUDE = $(shell python $(RAVEN_DIR)/scripts/find_numpy_include.py include_numpy)

DISTRIBUTION_COMPILE_COMMAND=@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) -I$(RAVEN_LIB_INCLUDE_DIR) -I$(RAVEN_DIR)/include/distributions/ -I$(RAVEN_DIR)/include/utilities/  -MMD -MF $@.d -MT $@ -c $< -o $@

#include $(PETSC_DIR)/conf/variables
EXTRA_MODULE_LIBS= #$(BLASLAPACK_LIB)

$(RAVEN_DIR)/src/distributions/DistributionContainer.$(obj-suffix): $(RAVEN_DIR)/src/distributions/DistributionContainer.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/distributions/distribution_1D.$(obj-suffix): $(RAVEN_DIR)/src/distributions/distribution_1D.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/distributions/distribution.$(obj-suffix): $(RAVEN_DIR)/src/distributions/distribution.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/distributions/distributionFunctions.$(obj-suffix): $(RAVEN_DIR)/src/distributions/distributionFunctions.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/distributions/distribution_base_ND.$(obj-suffix): $(RAVEN_DIR)/src/distributions/distribution_base_ND.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix): $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix): $(RAVEN_DIR)/src/utilities/NDspline.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix): $(RAVEN_DIR)/src/utilities/microSphere.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix): $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix): $(RAVEN_DIR)/src/utilities/MDreader.C
	$(DISTRIBUTION_COMPILE_COMMAND)

$(RAVEN_DIR)/python_modules/_distribution1Dpy2.so : $(RAVEN_DIR)/python_modules/distribution1Dpy2.i \
                                                 $(RAVEN_DIR)/src/distributions/distribution_1D.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/distributions/DistributionContainer.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/distributions/distributionFunctions.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/distributions/distribution_base_ND.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/distributions/distribution.$(obj-suffix)
# Swig
	swig -c++ -python  -I$(RAVEN_DIR)/include/distributions/ -I$(RAVEN_DIR)/include/utilities/ \
          $(RAVEN_PMODULES)/distribution1Dpy2.i
# Compile
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile \
	$(libmesh_CXX) $(libmesh_CPPFLAGS) $(PYTHON2_INCLUDE)\
         -I$(RAVEN_DIR)/include/distributions/ -I$(RAVEN_DIR)/include/utilities/ \
	 -c  $(RAVEN_PMODULES)/distribution1Dpy2_wrap.cxx -o $(RAVEN_DIR)/python_modules/distribution1Dpy2_wrap.lo
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link \
	 $(libmesh_CXX) $(libmesh_CXXFLAGS) \
	-shared -o $(RAVEN_PMODULES)/libdistribution1Dpy2.la $(PYTHON2_LIB) $(RAVEN_PMODULES)/distribution1Dpy2_wrap.lo $(RAVEN_DIR)/src/distributions/distribution_1D.$(obj-suffix) $(RAVEN_DIR)/src/distributions/distributionFunctions.$(obj-suffix)  $(RAVEN_DIR)/src/distributions/distribution.$(obj-suffix) $(RAVEN_DIR)/src/distributions/DistributionContainer.$(obj-suffix) $(RAVEN_DIR)/src/distributions/distribution_base_ND.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix) $(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix) $(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix) -rpath $(RAVEN_PMODULES) $(EXTRA_MODULE_LIBS)
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=install install -c $(RAVEN_PMODULES)/libdistribution1Dpy2.la  $(RAVEN_PMODULES)/libdistribution1Dpy2.la
	rm -f $(RAVEN_PMODULES)/_distribution1Dpy2.so
	ln -s libdistribution1Dpy2.$(raven_shared_ext) $(RAVEN_PMODULES)/_distribution1Dpy2.so

$(RAVEN_DIR)/python_modules/_distribution1Dpy3.so : $(RAVEN_DIR)/python_modules/distribution1Dpy3.i \
                                                 $(RAVEN_DIR)/src/distributions/distribution_1D.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/distributions/DistributionContainer.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/distributions/distributionFunctions.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/distributions/distribution_base_ND.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/distributions/distribution.$(obj-suffix)
# Swig
	swig -c++ -python -py3 -I$(RAVEN_DIR)/include/distributions/  -I$(RAVEN_DIR)/include/utilities/ \
          $(RAVEN_PMODULES)/distribution1Dpy3.i
# Compile
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile \
	$(libmesh_CXX) $(libmesh_CPPFLAGS) $(PYTHON_INCLUDE)\
         -I$(RAVEN_DIR)/include/distributions/ -I$(RAVEN_DIR)/include/utilities/ \
	 -c  $(RAVEN_PMODULES)/distribution1Dpy3_wrap.cxx -o $(RAVEN_DIR)/python_modules/distribution1Dpy3_wrap.lo
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link \
	 $(libmesh_CXX) $(libmesh_CXXFLAGS) \
	-shared -o $(RAVEN_PMODULES)/libdistribution1Dpy3.la $(PYTHON_LIB) $(RAVEN_PMODULES)/distribution1Dpy3_wrap.lo $(RAVEN_DIR)/src/distributions/distribution_1D.$(obj-suffix) $(RAVEN_DIR)/src/distributions/distributionFunctions.$(obj-suffix)  $(RAVEN_DIR)/src/distributions/distribution.$(obj-suffix) $(RAVEN_DIR)/src/distributions/DistributionContainer.$(obj-suffix) $(RAVEN_DIR)/src/distributions/distribution_base_ND.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix) $(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix) $(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix) -rpath $(RAVEN_PMODULES) $(EXTRA_MODULE_LIBS)
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=install install -c $(RAVEN_PMODULES)/libdistribution1Dpy3.la  $(RAVEN_PMODULES)/libdistribution1Dpy3.la
	rm -f $(RAVEN_PMODULES)/_distribution1Dpy3.so
	ln -s libdistribution1Dpy3.$(raven_shared_ext) $(RAVEN_PMODULES)/_distribution1Dpy3.so

######## Swig-ing ND_Interpolation_only

NDINTERPOLATION_COMPILE_COMMAND=@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) -I$(RAVEN_LIB_INCLUDE_DIR) -I$(RAVEN_DIR)/include/utilities/  -MMD -MF $@.d -MT $@ -c $< -o $@

#$(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix): $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.C
#	$(NDINTERPOLATION_COMPILE_COMMAND)

#$(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix): $(RAVEN_DIR)/src/utilities/NDspline.C
#	$(NDINTERPOLATION_COMPILE_COMMAND)

#$(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix): $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.C
#	$(NDINTERPOLATION_COMPILE_COMMAND)

#$(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix): $(RAVEN_DIR)/src/utilities/MDreader.C
#	$(NDINTERPOLATION_COMPILE_COMMAND)

$(RAVEN_DIR)/python_modules/_interpolationNDpy2.so : $(RAVEN_DIR)/python_modules/interpolationNDpy2.i \
                                                 $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix)
# Swig
	swig -c++ -python  -I$(RAVEN_DIR)/include/utilities/  \
          $(RAVEN_PMODULES)/interpolationNDpy2.i
# Compile
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile \
	$(libmesh_CXX) $(libmesh_CPPFLAGS) $(PYTHON2_INCLUDE)\
         -I$(RAVEN_DIR)/include/utilities/ \
	 -c  $(RAVEN_PMODULES)/interpolationNDpy2_wrap.cxx -o $(RAVEN_DIR)/python_modules/interpolationNDpy2_wrap.lo
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link \
	 $(libmesh_CXX) $(libmesh_CXXFLAGS) \
	-shared -o $(RAVEN_PMODULES)/libinterpolationNDpy2.la $(PYTHON2_LIB) $(RAVEN_PMODULES)/interpolationNDpy2_wrap.lo $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix) $(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix) $(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix) -rpath $(RAVEN_PMODULES) $(EXTRA_MODULE_LIBS)
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=install install -c $(RAVEN_PMODULES)/libinterpolationNDpy2.la  $(RAVEN_PMODULES)/libinterpolationNDpy2.la
	rm -f $(RAVEN_PMODULES)/_interpolationNDpy2.so
	ln -s libinterpolationNDpy2.$(raven_shared_ext) $(RAVEN_PMODULES)/_interpolationNDpy2.so

$(RAVEN_DIR)/python_modules/_interpolationNDpy3.so : $(RAVEN_DIR)/python_modules/interpolationNDpy3.i \
                                                 $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix) \
                                                 $(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix)
# Swig
	swig -c++ -python -py3 -I$(RAVEN_DIR)/include/utilities/  \
          $(RAVEN_PMODULES)/interpolationNDpy3.i
# Compile
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile \
	$(libmesh_CXX) $(libmesh_CPPFLAGS) $(PYTHON_INCLUDE)\
         -I$(RAVEN_DIR)/include/utilities/ \
	 -c  $(RAVEN_PMODULES)/interpolationNDpy3_wrap.cxx -o $(RAVEN_DIR)/python_modules/interpolationNDpy3_wrap.lo
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link \
	 $(libmesh_CXX) $(libmesh_CXXFLAGS) \
	-shared -o $(RAVEN_PMODULES)/libinterpolationNDpy3.la $(PYTHON_LIB) $(RAVEN_PMODULES)/interpolationNDpy3_wrap.lo $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.$(obj-suffix) $(RAVEN_DIR)/src/utilities/microSphere.$(obj-suffix) $(RAVEN_DIR)/src/utilities/NDspline.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.$(obj-suffix)  $(RAVEN_DIR)/src/utilities/MDreader.$(obj-suffix) -rpath $(RAVEN_PMODULES) $(EXTRA_MODULE_LIBS)
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=install install -c $(RAVEN_PMODULES)/libinterpolationNDpy3.la  $(RAVEN_PMODULES)/libinterpolationNDpy3.la
	rm -f $(RAVEN_PMODULES)/_interpolationNDpy3.so
	ln -s libinterpolationNDpy3.$(raven_shared_ext) $(RAVEN_PMODULES)/_interpolationNDpy3.so



