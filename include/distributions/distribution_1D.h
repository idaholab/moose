/*
 * distribution_1D.h
 *
 *  Created on: Mar 22, 2012
 *      Author: MANDD
 *      References:
 *
 *      Tests      : None for the custom
 *
 *      Problems   : Gamma, Beta and Custom distributions have no RNG in place yet
 *      Issues      : None
 *      Complaints   : None
 *      Compliments   : None
 *
 */

#ifndef DISTRIBUTION_1D_H_
#define DISTRIBUTION_1D_H_

#include <string>
#include <vector>
//#include "Interpolation_Functions.h"
#include "distribution.h"
#include "distributionFunctions.h"

class DistributionBackend;

class BasicTruncatedDistribution : public virtual BasicDistribution {
public:
  /*BasicTruncatedDistribution();
    virtual ~BasicTruncatedDistribution();*/
  virtual double  Pdf(double x);        ///< Pdf function at coordinate x
  virtual double  Cdf(double x);        ///< Cdf function at coordinate x
  virtual double  InverseCdf(double x); ///< x

  virtual double untrPdf(double x);
  virtual double untrCdf(double x);
  virtual double untrCdfComplement(double x);
  virtual double untrInverseCdf(double x);
  virtual double untrMean();
  virtual double untrMedian();
  virtual double untrMode();
  virtual double untrHazard(double x);
protected: 
  DistributionBackend * _backend;
};

class BasicDiscreteDistribution : public virtual BasicDistribution {
public:
  /*BasicTruncatedDistribution();
    virtual ~BasicTruncatedDistribution();*/
  virtual double  Pdf(double x);           ///< Pdf function at coordinate x
  virtual double  Cdf(double x);           ///< Cdf function at coordinate x
  virtual double  InverseCdf(double x);    ///< x

  virtual double untrPdf(double x);
  virtual double untrCdf(double x);
  virtual double untrCdfComplement(double x);
  virtual double untrInverseCdf(double x);
  virtual double untrMean();
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

class BasicUniformDistribution : public BasicTruncatedDistribution {
public:
  BasicUniformDistribution(double xMin, double xMax);
  virtual ~BasicUniformDistribution();
   double  Pdf(double x);                ///< Pdf function at coordinate x
   double  Cdf(double x);                ///< Cdf function at coordinate x
   double  InverseCdf(double x);        ///< x

};


/*
 * CLASS NORMAL DISTRIBUTION
 */
class NormalDistribution;


class NormalDistributionBackend;

class BasicNormalDistribution : public  BasicTruncatedDistribution {
public:
   BasicNormalDistribution(double mu, double sigma);
   BasicNormalDistribution(double mu, double sigma, double xMin, double xMax);
   virtual ~BasicNormalDistribution();

   double  InverseCdf(double x);        ///< x

};


/*
 * CLASS LOG NORMAL DISTRIBUTION
 */
class LogNormalDistribution;

class LogNormalDistributionBackend;

class BasicLogNormalDistribution : public BasicTruncatedDistribution {
public:
  BasicLogNormalDistribution(double mu, double sigma);
  virtual ~BasicLogNormalDistribution();
  
  double  InverseCdf(double x);        ///< x

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
   virtual ~BasicLogisticDistribution();

};


/*
 * CLASS TRIANGULAR DISTRIBUTION
 */

class TriangularDistribution;

class TriangularDistributionBackend;

class BasicTriangularDistribution : public BasicTruncatedDistribution {
public:
   BasicTriangularDistribution(double xPeak, double lowerBound, double upperBound);
   virtual ~BasicTriangularDistribution();

};


/*
 * CLASS EXPONENTIAL DISTRIBUTION
 */

class ExponentialDistribution;

class ExponentialDistributionBackend;

class BasicExponentialDistribution : public BasicTruncatedDistribution {
public:
   BasicExponentialDistribution(double lambda);
   virtual ~BasicExponentialDistribution();

   double  untrCdf(double x);
};

/*
 * CLASS WEIBULL DISTRIBUTION
 */

class WeibullDistribution;

class WeibullDistributionBackend;

class BasicWeibullDistribution : public BasicTruncatedDistribution {
public:
   BasicWeibullDistribution(double k, double lambda);
   virtual ~BasicWeibullDistribution();

   double  untrCdf(double x);
};

/*
 * CLASS GAMMA DISTRIBUTION
 */

class GammaDistributionBackend;

class BasicGammaDistribution : public BasicTruncatedDistribution {
public:
  BasicGammaDistribution(double k, double theta, double low);
  virtual ~BasicGammaDistribution();

  double  untrPdf(double x);                ///< Pdf function at coordinate x
  double  untrCdf(double x);                ///< Cdf function at coordinate x
  double  untrInverseCdf(double x);        ///< x
  
};

/*
 * CLASS BETA DISTRIBUTION
 */

class BetaDistributionBackend;

class BasicBetaDistribution : public BasicTruncatedDistribution {
public:
  BasicBetaDistribution(double alpha, double beta, double scale);
  virtual ~BasicBetaDistribution();

  double  Pdf(double x);                ///< Pdf function at coordinate x
  double  Cdf(double x);                ///< Cdf function at coordinate x
  double  InverseCdf(double x);        ///< x
  
  double  untrCdf(double x);
};

/*
 * CLASS POISSON DISTRIBUTION
 */

class PoissonDistributionBackend;

class BasicPoissonDistribution : public BasicDiscreteDistribution {
public:
  BasicPoissonDistribution(double mu);
  virtual ~BasicPoissonDistribution();

  double  Pdf(double x);                ///< Pdf function at coordinate x
  double  Cdf(double x);                ///< Cdf function at coordinate x
  double  InverseCdf(double x);        ///< x
  
  double  untrCdf(double x);
};

/*
 * CLASS BINOMIAL DISTRIBUTION
 */

class BinomialDistributionBackend;

class BasicBinomialDistribution : public BasicDiscreteDistribution {
public:
  BasicBinomialDistribution(double n, double p);
  virtual ~BasicBinomialDistribution();
 
  double  untrCdf(double x);
};

/*
 * CLASS BERNOULLI DISTRIBUTION
 */

class BernoulliDistributionBackend;

class BasicBernoulliDistribution : public BasicDiscreteDistribution {
public:
  BasicBernoulliDistribution(double p);
  virtual ~BasicBernoulliDistribution();
  
};


class BasicConstantDistribution : public virtual BasicDistribution {
public:
  BasicConstantDistribution(double value);
  virtual ~BasicConstantDistribution();
  double  Pdf(double x);           ///< Pdf function at coordinate x
  double  Cdf(double x);           ///< Cdf function at coordinate x
  double  InverseCdf(double);    ///< x

  double untrPdf(double x);
  double untrCdf(double x);
  double untrCdfComplement(double x);
  double untrInverseCdf(double);
  double untrMean();
  double untrMedian();
  double untrMode();
  double untrHazard(double x);
protected: 
  double _value;
};

/*
 * CLASS CUSTOM DISTRIBUTION
 */

// class CustomDistribution;

// class BasicCustomDistribution : public virtual BasicDistribution {
// public:
//    BasicCustomDistribution(double x_coordinates, double y_coordinates, int fitting_type, double n_points);
//    virtual ~BasicCustomDistribution();

//    double  Pdf(double x);                ///< Pdf function at coordinate x
//    double  Cdf(double x);                ///< Cdf function at coordinate x
//    double  InverseCdf(double x);        ///< x

// protected:
// };

#endif
