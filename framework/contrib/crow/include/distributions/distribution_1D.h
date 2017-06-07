/* Copyright 2017 Battelle Energy Alliance, LLC

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#ifndef DISTRIBUTION_1D_H
#define DISTRIBUTION_1D_H

#include <string>
#include <vector>
#include "distribution.h"
#include "distributionFunctions.h"

class DistributionBackend;

class BasicTruncatedDistribution : public virtual BasicDistribution
{
public:
  BasicTruncatedDistribution(double x_min, double x_max);
  BasicTruncatedDistribution(){};
  virtual double pdf(double x);        ///< pdf function at coordinate x
  virtual double cdf(double x);        ///< cdf function at coordinate x
  virtual double inverseCdf(double x); ///< x

  virtual double untrPdf(double x);
  virtual double untrCdf(double x);
  virtual double untrCdfComplement(double x);
  virtual double untrInverseCdf(double x);
  virtual double untrMean();
  virtual double untrStdDev();
  virtual double untrMedian();
  virtual double untrMode();
  virtual double untrHazard(double x);
protected:
  DistributionBackend * _backend;
};

class BasicDiscreteDistribution : public virtual BasicDistribution
{
public:
  virtual double pdf(double x);           ///< pdf function at coordinate x
  virtual double cdf(double x);           ///< cdf function at coordinate x
  virtual double inverseCdf(double x);    ///< x

  virtual double untrPdf(double x);
  virtual double untrCdf(double x);
  virtual double untrCdfComplement(double x);
  virtual double untrInverseCdf(double x);
  virtual double untrMean();
  virtual double untrStdDev();
  virtual double untrMedian();
  virtual double untrMode();
  virtual double untrHazard(double x);
protected:
  DistributionBackend * _backend;
};

/*
 * CLASS UNIFORM DISTRIBUTION
 */
class UniformDistribution;


class UniformDistributionBackend;

class BasicUniformDistribution : public BasicTruncatedDistribution
{
public:
  BasicUniformDistribution(double x_min, double x_max);
  virtual ~BasicUniformDistribution();
  double  pdf(double x);                ///< pdf function at coordinate x
  double  cdf(double x);                ///< cdf function at coordinate x
  double  inverseCdf(double x);        ///< x
};


/*
 * CLASS NORMAL DISTRIBUTION
 */
class NormalDistribution;


class NormalDistributionBackend;

class BasicNormalDistribution : public  BasicTruncatedDistribution
{
public:
  BasicNormalDistribution(double mu, double sigma);
  BasicNormalDistribution(double mu, double sigma, double x_min, double x_max);
  virtual ~BasicNormalDistribution();

  double  inverseCdf(double x);        ///< x
};


/*
 * CLASS LOG NORMAL DISTRIBUTION
 */
class LogNormalDistribution;

class LogNormalDistributionBackend;

class BasicLogNormalDistribution : public BasicTruncatedDistribution
{
public:
  BasicLogNormalDistribution(double mu, double sigma, double low);
  BasicLogNormalDistribution(double mu, double sigma, double x_min, double x_max, double low);
  virtual ~BasicLogNormalDistribution();

  double  inverseCdf(double x);        ///< x

  double untrPdf(double x);
  double untrCdf(double x);
};

/*
 * CLASS LOGISTIC DISTRIBUTION
 */
class LogisticDistribution;


class LogisticDistributionBackend;

class BasicLogisticDistribution : public BasicTruncatedDistribution {
public:
   BasicLogisticDistribution(double location, double scale);
   BasicLogisticDistribution(double location, double scale, double x_min, double x_max);
   virtual ~BasicLogisticDistribution();

};

/*
 * CLASS LAPLACE DISTRIBUTION
 */
class LaplaceDistribution;

class LaplaceDistributionBackend;

class BasicLaplaceDistribution : public BasicTruncatedDistribution {
public:
  BasicLaplaceDistribution(double location, double scale, double x_min, double x_max);
  virtual ~BasicLaplaceDistribution();
};


/*
 * CLASS TRIANGULAR DISTRIBUTION
 */

class TriangularDistribution;

class TriangularDistributionBackend;

class BasicTriangularDistribution : public BasicTruncatedDistribution
{
public:
  BasicTriangularDistribution(double x_peak, double lower_bound, double upper_bound);
  virtual ~BasicTriangularDistribution();

};


/*
 * CLASS EXPONENTIAL DISTRIBUTION
 */

class ExponentialDistribution;

class ExponentialDistributionBackend;

class BasicExponentialDistribution : public BasicTruncatedDistribution
{
public:
  BasicExponentialDistribution(double lambda, double low);
  BasicExponentialDistribution(double lambda, double x_min, double x_max, double low);
  virtual ~BasicExponentialDistribution();

  double untrPdf(double x);
  double untrCdf(double x);
  double cdf(double x);
  double inverseCdf(double x);
};

/*
 * CLASS WEIBULL DISTRIBUTION
 */

class WeibullDistribution;

class WeibullDistributionBackend;

class BasicWeibullDistribution : public BasicTruncatedDistribution
{
public:
  BasicWeibullDistribution(double k, double lambda, double low);
  BasicWeibullDistribution(double k, double lambda, double x_min, double x_max, double low);
  virtual ~BasicWeibullDistribution();

  double  untrPdf(double x);
  double  untrCdf(double x);
  double inverseCdf(double x);
};

/*
 * CLASS GAMMA DISTRIBUTION
 */

class GammaDistributionBackend;

class BasicGammaDistribution : public BasicTruncatedDistribution
{
public:
  BasicGammaDistribution(double k, double theta, double low);
  BasicGammaDistribution(double k, double theta, double low, double x_min, double x_max);
  virtual ~BasicGammaDistribution();

  double  untrPdf(double x);                ///< pdf function at coordinate x
  double  untrCdf(double x);                ///< cdf function at coordinate x
  double  untrInverseCdf(double x);        ///< x

};

/*
 * CLASS BETA DISTRIBUTION
 */

class BetaDistributionBackend;

class BasicBetaDistribution : public BasicTruncatedDistribution
{
public:
  BasicBetaDistribution(double alpha, double beta, double scale, double low);
  BasicBetaDistribution(double alpha, double beta, double scale, double x_min, double x_max, double low);
  virtual ~BasicBetaDistribution();

  double  pdf(double x);                ///< pdf function at coordinate x
  double  cdf(double x);                ///< cdf function at coordinate x
  double  inverseCdf(double x);        ///< x

  double  untrCdf(double x);
  double  untrPdf(double x);
};

/*
 * CLASS POISSON DISTRIBUTION
 */

class PoissonDistributionBackend;

class BasicPoissonDistribution : public BasicDiscreteDistribution
{
public:
  BasicPoissonDistribution(double mu);
  virtual ~BasicPoissonDistribution();

  double  pdf(double x);                ///< pdf function at coordinate x
  double  cdf(double x);                ///< cdf function at coordinate x
  double  inverseCdf(double x);        ///< x

  double  untrCdf(double x);
};

/*
 * CLASS BINOMIAL DISTRIBUTION
 */

class BinomialDistributionBackend;

class BasicBinomialDistribution : public BasicDiscreteDistribution
{
public:
  BasicBinomialDistribution(double n, double p);
  virtual ~BasicBinomialDistribution();

  double  untrCdf(double x);
};

/*
 * CLASS BERNOULLI DISTRIBUTION
 */

class BernoulliDistributionBackend;

class BasicBernoulliDistribution : public BasicDiscreteDistribution
{
public:
  BasicBernoulliDistribution(double p);
  virtual ~BasicBernoulliDistribution();
};

/*
 * CLASS GEOMETRIC DISTRIBUTION
 */
class GeometricDistribution;

class GeometricDistributionBackend;

class BasicGeometricDistribution : public BasicDiscreteDistribution {
public:
  BasicGeometricDistribution(double p);
  virtual ~BasicGeometricDistribution();
};

/*
 * CLASS CONSTANT DISTRIBUTION
 */


class BasicConstantDistribution : public virtual BasicDistribution
{
public:
  BasicConstantDistribution(double value);
  virtual ~BasicConstantDistribution();
  double  pdf(double x);           ///< pdf function at coordinate x
  double  cdf(double x);           ///< cdf function at coordinate x
  double  inverseCdf(double);    ///< x

  double untrPdf(double x);
  double untrCdf(double x);
  double untrCdfComplement(double x);
  double untrInverseCdf(double);
  double untrMean();
  double untrStdDev();
  double untrMedian();
  double untrMode();
  double untrHazard(double x);
protected:
  double _value;
};


#endif
