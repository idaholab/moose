#For future compatibility with Python 3
from __future__ import division, print_function, unicode_literals, absolute_import
import warnings
warnings.simplefilter('default',DeprecationWarning)

import sys
import utils

distribution1D = utils.find_distribution1D()

normal_distribution = distribution1D.BasicNormalDistribution(1.0,2.0,-sys.float_info.max, sys.float_info.max)

results = {"pass":0,"fail":0}

utils.checkAnswer("normal cdf(0.0)",normal_distribution.cdf(0.0),0.308537538726,results)
utils.checkAnswer("normal cdf(1.0)",normal_distribution.cdf(1.0),0.5,results)
utils.checkAnswer("normal cdf(2.0)",normal_distribution.cdf(2.0),0.691462461274,results)

utils.checkAnswer("normal mean",normal_distribution.untrMean(),1.0,results)
utils.checkAnswer("normal stddev",normal_distribution.untrStdDev(),2.0,results)
utils.checkAnswer("normal ppf(0.1)",normal_distribution.inverseCdf(0.1),-1.56310313109,results)
utils.checkAnswer("normal ppf(0.5)",normal_distribution.inverseCdf(0.5),1.0,results)
utils.checkAnswer("normal ppf(0.9)",normal_distribution.inverseCdf(0.9),3.56310313109,results)
utils.checkAnswer("normal mean()",normal_distribution.untrMean(),1.0,results)
utils.checkAnswer("normal median()",normal_distribution.untrMedian(),1.0,results)
utils.checkAnswer("normal mode()",normal_distribution.untrMode(),1.0,results)


print(results)

sys.exit(results["fail"])
