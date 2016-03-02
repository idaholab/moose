from distutils.core import setup, Extension
include_dirs=['include/distributions','include/utilities','contrib/include']
swig_opts=['-c++','-py3','-Iinclude/distributions','-Iinclude/utilities']
setup(name='crow',
      version='0.8',
      ext_package='crow_modules',
      ext_modules=[Extension('_distribution1Dpy3',['crow_modules/distribution1Dpy3.i','src/distributions/distribution.C','src/utilities/MDreader.C','src/utilities/inverseDistanceWeigthing.C','src/utilities/microSphere.C','src/utilities/NDspline.C','src/utilities/ND_Interpolation_Functions.C','src/distributions/distributionNDBase.C','src/distributions/distributionNDNormal.C','src/distributions/distributionFunctions.C','src/distributions/DistributionContainer.C','src/distributions/distribution_1D.C','src/distributions/randomClass.C','src/distributions/distributionNDCartesianSpline.C'],include_dirs=include_dirs,swig_opts=swig_opts),
                   Extension('_interpolationNDpy3',['crow_modules/interpolationNDpy3.i','src/utilities/ND_Interpolation_Functions.C','src/utilities/NDspline.C','src/utilities/microSphere.C','src/utilities/inverseDistanceWeigthing.C','src/utilities/MDreader.C','src/distributions/randomClass.C'],include_dirs=include_dirs,swig_opts=swig_opts)],
      py_modules=['crow_modules.distribution1Dpy3','crow_modules.interpolationNDpy3'],
      )
