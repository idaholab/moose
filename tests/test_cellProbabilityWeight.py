#For future compatibility with Python 3
from __future__ import division, print_function, unicode_literals, absolute_import
import warnings
warnings.simplefilter('default',DeprecationWarning)

import sys
import utils

distribution1D = utils.find_distribution1D()
# input data, random matrix can also be used.
mu = [1.0,2.0,3.0,4.0,5.0]
cov = [1.36,   -0.816,  0.521,  1.43,    -0.144,
       -0.816, -0.659,  0.794,  -0.173,  -0.406,
       0.521,  0.794,   -0.541, 0.461,   0.179,
       1.43,   -0.173,  0.461,  -1.43,   0.822,
       -0.144, -0.406,  0.179,  0.822,   -1.37]

# Transform 'mu' and 'cov' to the c++ vector
muCpp = distribution1D.vectord_cxx(len(mu))
for i in range(len(mu)):
  muCpp[i] = mu[i]
covCpp = distribution1D.vectord_cxx(len(cov))
for i in range(len(cov)):
  covCpp[i] = cov[i]

# call the functions from the crow to compute the svd
covType = "abs"
rank = 5
mvnDistribution = distribution1D.BasicMultivariateNormal(covCpp,muCpp,str(covType),rank)

results = {"pass":0,"fail":0}

x0 = [-2.0,-2.0,-2.0,-2.0,-2.0]
dx = [0.5,0.5,0.5,0.5,0.5]
center = distribution1D.vectord_cxx(len(x0))
dx_center = distribution1D.vectord_cxx(len(dx))
for i in range(len(mu)):
  center[i] = x0[i]
  dx_center[i] = dx[i]
utils.checkAnswer("multivariate norm cellProbabilityWeight (center,dx_center)",mvnDistribution.cellProbabilityWeight(center,dx_center),1.6708270299032051e-08,results)

x0 = [-1.0,-1.0,-1.0,-1.0,-1.0]
dx = [0.5,0.5,0.5,0.5,0.5]
center = distribution1D.vectord_cxx(len(x0))
dx_center = distribution1D.vectord_cxx(len(dx))
for i in range(len(mu)):
  center[i] = x0[i]
  dx_center[i] = dx[i]
utils.checkAnswer("multivariate norm cellProbabilityWeight (center,dx_center)",mvnDistribution.cellProbabilityWeight(center,dx_center),2.5913402492078289e-05,results)

x0 = [0.0,0.0,0.0,0.0,0.0]
dx = [0.5,0.5,0.5,0.5,0.5]
center = distribution1D.vectord_cxx(len(x0))
dx_center = distribution1D.vectord_cxx(len(dx))
for i in range(len(mu)):
  center[i] = x0[i]
  dx_center[i] = dx[i]
utils.checkAnswer("multivariate norm cellProbabilityWeight (center,dx_center)",mvnDistribution.cellProbabilityWeight(center,dx_center),2.9982987719114122e-04,results)

x0 = [-2.0,-1.0,0.0,1.0,2.0]
dx = [0.5,0.5,0.5,0.5,0.5]
center = distribution1D.vectord_cxx(len(x0))
dx_center = distribution1D.vectord_cxx(len(dx))
for i in range(len(mu)):
  center[i] = x0[i]
  dx_center[i] = dx[i]
utils.checkAnswer("multivariate norm cellProbabilityWeight (center,dx_center)",mvnDistribution.cellProbabilityWeight(center,dx_center),2.2385013190007662e-06,results)

x0 = [-1.0,0.0,1.0,2.0,3.0]
dx = [0.1,0.2,0.3,0.4,0.5]
center = distribution1D.vectord_cxx(len(x0))
dx_center = distribution1D.vectord_cxx(len(dx))
for i in range(len(mu)):
  center[i] = x0[i]
  dx_center[i] = dx[i]
utils.checkAnswer("multivariate norm cellProbabilityWeight (center,dx_center)",mvnDistribution.cellProbabilityWeight(center,dx_center),7.4049000729169408e-09,results)

print(results)
sys.exit(results["fail"])
