# Copyright 2017 Battelle Energy Alliance, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from distutils.core import setup, Extension
import subprocess
try:
  eigen_flags = subprocess.check_output(["./scripts/find_eigen.py"]).decode("ascii")
except:
  eigen_flags = ""
include_dirs=['include/distributions','include/utilities','contrib/include']
if eigen_flags.startswith("-I"):
  include_dirs.append(eigen_flags[2:].rstrip())
swig_opts=['-c++','-py3','-Iinclude/distributions','-Iinclude/utilities']
extra_compile_args=['-std=c++11']
ext = 'py3'
setup(name='crow',
      version='0.8',
      ext_package='crow_modules',
      ext_modules=[Extension('_distribution1D'+ext,['crow_modules/distribution1D'+ext+'.i','src/distributions/distribution.cxx','src/utilities/MDreader.cxx','src/utilities/inverseDistanceWeigthing.cxx','src/utilities/microSphere.cxx','src/utilities/NDspline.cxx','src/utilities/ND_Interpolation_Functions.cxx','src/distributions/distributionNDBase.cxx','src/distributions/distributionNDNormal.cxx','src/distributions/distributionFunctions.cxx','src/distributions/DistributionContainer.cxx','src/distributions/distribution_1D.cxx','src/distributions/randomClass.cxx','src/distributions/distributionNDCartesianSpline.cxx'],include_dirs=include_dirs,swig_opts=swig_opts,extra_compile_args=extra_compile_args),
                   Extension('_interpolationND'+ext,['crow_modules/interpolationND'+ext+'.i','src/utilities/ND_Interpolation_Functions.cxx','src/utilities/NDspline.cxx','src/utilities/microSphere.cxx','src/utilities/inverseDistanceWeigthing.cxx','src/utilities/MDreader.cxx','src/distributions/randomClass.cxx'],include_dirs=include_dirs,swig_opts=swig_opts,extra_compile_args=extra_compile_args)],
      py_modules=['crow_modules.distribution1D'+ext,'crow_modules.interpolationND'+ext],
      )
