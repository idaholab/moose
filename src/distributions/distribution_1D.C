/*
 * distribution_1D.cpp
 *
 *  Created on: Mar 22, 2012
 *      Author: MANDD
 *      Modified: alfoa
 *      References:
 *      1- G. Cassella, R.G. Berger, "Statistical Inference", 2nd ed. Pacific Grove, CA: Duxbury Press (2001).
 *
 */

#include "distribution_1D.h"
#include "distributionFunctions.h"
#include <math.h>
#include <cmath>               // needed to use erfc error function
#include <string>
#include "dynamicArray.h"
#include <ctime>
#include <cstdlib>
//#include "Interpolation_Functions.h"
#include <string>
#include <limits>
#include <boost/math/distributions/uniform.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/lognormal.hpp>
#include <boost/math/distributions/triangular.hpp>
#include <boost/math/distributions/exponential.hpp>
#include <boost/math/distributions/weibull.hpp>
#include <boost/math/distributions/gamma.hpp>
#include <boost/math/distributions/beta.hpp>
#include <boost/math/distributions/poisson.hpp>
#include <boost/math/distributions/binomial.hpp>
#include <boost/math/distributions/logistic.hpp>
#include <boost/math/distributions/bernoulli.hpp>

#define _USE_MATH_DEFINES   // needed in order to use M_PI = 3.14159

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

class DistributionBackend {
public:
  virtual double pdf(double x) = 0;
  virtual double cdf(double x) = 0;
  virtual double cdfComplement(double x) = 0;
  virtual double quantile(double x) = 0;
  virtual double mean() = 0;
  virtual double median() = 0;
  virtual double mode() = 0;
  virtual double hazard(double x) = 0;
  virtual ~DistributionBackend() {};
};

/*
 * Class Basic Truncated Distribution
 * This class implements a basic truncated distribution that can
 * be inherited from.
 */

double
BasicTruncatedDistribution::Pdf(double x){
  double value;
  double xMin = _dis_parameters.find("xMin") ->second;
  double xMax = _dis_parameters.find("xMax") ->second;

  if (_dis_parameters.find("truncation") ->second == 1) {
    if ((x<xMin)||(x>xMax)) {
      value=0;
    } else {
      value = 1/(untrCdf(xMax) - untrCdf(xMin)) * untrPdf(x);
    }
  } else {
    value=-1;
  }

  return value;
}

double
BasicTruncatedDistribution::Cdf(double x){
  double value;
  double xMin = _dis_parameters.find("xMin") ->second;
  double xMax = _dis_parameters.find("xMax") ->second;

  if (_dis_parameters.find("truncation") ->second == 1) {
    if (x<xMin) {
      value=0;
    } else if (x>xMax) {
      value=1;
    } else{
      value = 1/(untrCdf(xMax) - untrCdf(xMin)) * (untrCdf(x)- untrCdf(xMin));
    }
  } else {
    value=-1;
  }

  return value;
}

double
BasicTruncatedDistribution::InverseCdf(double x){
  double value;
  double xMin = _dis_parameters.find("xMin") ->second;
  double xMax = _dis_parameters.find("xMax") ->second;
  if(x == 0.0) {
    //Using == in floats is generally a bad idea, but
    // 0.0 can be represented exactly.
    //In this case, return the minimum value
    return xMin;
  }
  if(x == 1.0) {
    //Using == in floats is generally a bad idea, but
    // 1.0 can be represented exactly.
    //In this case, return the maximum value
    return xMax;
  }
  if (_dis_parameters.find("truncation") ->second == 1){
    double temp=untrCdf(xMin)+x*(untrCdf(xMax)-untrCdf(xMin));
    value=untrInverseCdf(temp);
  } else {
    value=-1;
  }
  return value;
}


double BasicTruncatedDistribution::untrPdf(double x) {
  return _backend->pdf(x);
}

double BasicTruncatedDistribution::untrCdf(double x) {
  return _backend->cdf(x);
}

double BasicTruncatedDistribution::untrCdfComplement(double x) {
  return _backend->cdfComplement(x);
}

double BasicTruncatedDistribution::untrInverseCdf(double x) {
  return _backend->quantile(x);
}

double BasicTruncatedDistribution::untrMean() {
  return _backend->mean();
}

double BasicTruncatedDistribution::untrMedian() {
  return _backend->median();
}

double BasicTruncatedDistribution::untrMode() {
  return _backend->mode();
}

double BasicTruncatedDistribution::untrHazard(double x) {
  return _backend->hazard(x);
}



/*
 * Class Basic Discrete Distribution
 * This class implements a basic discrete distribution that can
 * be inherited from.
 */

double BasicDiscreteDistribution::untrPdf(double x) {
  return _backend->pdf(x);
}

double BasicDiscreteDistribution::untrCdf(double x) {
  return _backend->cdf(x);
}

double BasicDiscreteDistribution::untrCdfComplement(double x) {
  return _backend->cdfComplement(x);
}

double BasicDiscreteDistribution::untrInverseCdf(double x) {
  return _backend->quantile(x);
}

double BasicDiscreteDistribution::untrMean() {
  return _backend->mean();
}

double BasicDiscreteDistribution::untrMedian() {
  return _backend->median();
}

double BasicDiscreteDistribution::untrMode() {
  return _backend->mode();
}

double BasicDiscreteDistribution::untrHazard(double x) {
  return _backend->hazard(x);
}

double BasicDiscreteDistribution::Pdf(double x) {
  return untrPdf(x);
}

double BasicDiscreteDistribution::Cdf(double x) {
  return untrCdf(x);
}

double BasicDiscreteDistribution::InverseCdf(double x) {
  return untrInverseCdf(x);
}

/*
 * Class DistributionBackendTemplate implements a template that
 * can be used to create a DistributionBackend from a boost distribution
 */

template <class T>
class DistributionBackendTemplate : public DistributionBackend {
public:
  double pdf(double x) { return boost::math::pdf(*_backend, x); };
  double cdf(double x) { return boost::math::cdf(*_backend, x); };
  double cdfComplement(double x) { return boost::math::cdf(boost::math::complement(*_backend, x)); };
  double quantile(double x) { return boost::math::quantile(*_backend, x); };
  double mean() { return boost::math::mean(*_backend); };
  double median() { return boost::math::median(*_backend); };
  double mode() { return boost::math::mode(*_backend); };
  double hazard(double x) { return boost::math::hazard(*_backend, x); };
protected:
  T *_backend;
};

/*
 * CLASS UNIFORM DISTRIBUTION
 */


class UniformDistributionBackend : public DistributionBackendTemplate<boost::math::uniform> {
public:
  UniformDistributionBackend(double xMin, double xMax) {
    _backend = new boost::math::uniform(xMin,xMax);
  }
  ~UniformDistributionBackend() {
    delete _backend;
  }
};


BasicUniformDistribution::BasicUniformDistribution(double xMin, double xMax)
{
  _dis_parameters["xMin"] = xMin;
  _dis_parameters["xMax"] = xMax;
  _backend = new UniformDistributionBackend(xMin, xMax);

  if (xMin>xMax)
    throwError("ERROR: bounds for uniform distribution are incorrect");
}

BasicUniformDistribution::~BasicUniformDistribution()
{
  delete _backend;
}

double
BasicUniformDistribution::Pdf(double x){
  return untrPdf(x);
}

double
BasicUniformDistribution::Cdf(double x){
  return untrCdf(x);
}

double
BasicUniformDistribution::InverseCdf(double x){
  return untrInverseCdf(x);
}

class NormalDistributionBackend : public DistributionBackendTemplate<boost::math::normal> {
public:
  NormalDistributionBackend(double mean, double sd) {
    _backend = new boost::math::normal(mean, sd);
  }
  ~NormalDistributionBackend() {
    delete _backend;
  }
};

/*
 * CLASS NORMAL DISTRIBUTION
 */

BasicNormalDistribution::BasicNormalDistribution(double mu, double sigma) {
  _dis_parameters["mu"] = mu; //mean
  _dis_parameters["sigma"] = sigma; //sd
  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = -std::numeric_limits<double>::max( );
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = std::numeric_limits<double>::max( );
  }
  //std::cout << "mu " << mu << " sigma " << sigma
  //          << " truncation " << _dis_parameters["truncation"]
  //          << " xMin " << _dis_parameters["xMin"]
  //          << " xMax " << _dis_parameters["xMax"] << std::endl;
  _backend = new NormalDistributionBackend(mu, sigma);
}

BasicNormalDistribution::BasicNormalDistribution(double mu, double sigma, double xMin, double xMax) {
  _dis_parameters["mu"] = mu; //mean
  _dis_parameters["sigma"] = sigma; //sd
  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  _dis_parameters["xMin"] = xMin;
  _dis_parameters["xMax"] = xMax;
  //std::cout << "mu " << mu << " sigma " << sigma
  //          << " truncation " << _dis_parameters["truncation"]
  //          << " xMin " << _dis_parameters["xMin"]
  //          << " xMax " << _dis_parameters["xMax"] << std::endl;
  _backend = new NormalDistributionBackend(mu, sigma);

}


BasicNormalDistribution::~BasicNormalDistribution(){
  delete _backend;
}


double
BasicNormalDistribution::InverseCdf(double x){
  return BasicTruncatedDistribution::InverseCdf(x);
}

class LogNormalDistributionBackend : public DistributionBackendTemplate<boost::math::lognormal> {
public:
  LogNormalDistributionBackend(double mean, double sd) {
    _backend = new boost::math::lognormal(mean, sd);
  }
  ~LogNormalDistributionBackend() {
    delete  _backend;
  }
};

/*
 * CLASS LOG NORMAL DISTRIBUTION
 */


BasicLogNormalDistribution::BasicLogNormalDistribution(double mu, double sigma)
{
  _dis_parameters["mu"] = mu;
  _dis_parameters["sigma"] = sigma;

  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = 0.0;
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = std::numeric_limits<double>::max( );
  }

  if (mu<0)
    throwError("ERROR: incorrect value of mu for lognormaldistribution");

  _backend = new LogNormalDistributionBackend(mu, sigma);

}

BasicLogNormalDistribution::~BasicLogNormalDistribution()
{
  delete _backend;
}

double
BasicLogNormalDistribution::untrCdf(double x){
  //std::cout << "LogNormalDistribution::untrCdf " << x << std::endl;
  if(x <= 0) {
    return 0.0;
  } else {
    return _backend->cdf(x);
  }
}


double
BasicLogNormalDistribution::InverseCdf(double x){
  return BasicTruncatedDistribution::InverseCdf(x);
}

/*
 * CLASS LOGISTIC DISTRIBUTION
 */


class LogisticDistributionBackend : public DistributionBackendTemplate<boost::math::logistic_distribution<> > {
public:
  LogisticDistributionBackend(double location, double scale) {
    _backend = new boost::math::logistic_distribution<>(location, scale);
  }
  ~LogisticDistributionBackend() {
    delete _backend;
  }
};


BasicLogisticDistribution::BasicLogisticDistribution(double location, double scale)
{
  _dis_parameters["location"] = location;
  _dis_parameters["scale"] = scale;

  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = -std::numeric_limits<double>::max( );
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = std::numeric_limits<double>::max( );
  }

  _backend = new LogisticDistributionBackend(location, scale);
}

BasicLogisticDistribution::~BasicLogisticDistribution()
{
  delete _backend;
}

/*
 * CLASS TRIANGULAR DISTRIBUTION
 */



class TriangularDistributionBackend : public DistributionBackendTemplate<boost::math::triangular> {
public:
  TriangularDistributionBackend(double lower, double mode, double upper) {
    _backend = new boost::math::triangular(lower, mode, upper);
  }
  ~TriangularDistributionBackend() {
    delete _backend;
  }
};


BasicTriangularDistribution::BasicTriangularDistribution(double xPeak, double lowerBound, double upperBound)
{
  _dis_parameters["xPeak"] = xPeak;
  _dis_parameters["lowerBound"] = lowerBound;
  _dis_parameters["upperBound"] = upperBound;

  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = lowerBound;
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = upperBound;
  }


  if (upperBound < lowerBound)
    throwError("ERROR: bounds for triangular distribution are incorrect");
  if (upperBound < _dis_parameters.find("xMin") ->second)
    throwError("ERROR: bounds and LB/UB are inconsistent for triangular distribution");
  if (lowerBound > _dis_parameters.find("xMax") ->second)
    throwError("ERROR: bounds and LB/UB are inconsistent for triangular distribution");
  _backend = new TriangularDistributionBackend(lowerBound, xPeak, upperBound);

}
BasicTriangularDistribution::~BasicTriangularDistribution()
{
  delete _backend;
}


/*
 * CLASS EXPONENTIAL DISTRIBUTION
 */


class ExponentialDistributionBackend : public DistributionBackendTemplate<boost::math::exponential> {
public:
  ExponentialDistributionBackend(double lambda) {
    _backend = new boost::math::exponential(lambda);
  }
  ~ExponentialDistributionBackend() {
    delete _backend;
  }
};


BasicExponentialDistribution::BasicExponentialDistribution(double lambda)
{
  _dis_parameters["lambda"] = lambda;

  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = 0.0;
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = std::numeric_limits<double>::max( );
  }


  if (lambda<0)
    throwError("ERROR: incorrect value of lambda for exponential distribution");

  _backend = new ExponentialDistributionBackend(lambda);
}
BasicExponentialDistribution::~BasicExponentialDistribution()
{
  delete _backend;
}


double
BasicExponentialDistribution::untrCdf(double x){
  if(x >= 0.0) {
    return _backend->cdf(x);
  } else {
    return 0.0;
  }
}


/*
 * CLASS WEIBULL DISTRIBUTION
 */


class WeibullDistributionBackend : public DistributionBackendTemplate< boost::math::weibull>  {
public:
  WeibullDistributionBackend(double shape, double scale) {
    _backend = new boost::math::weibull(shape, scale);
  }
  ~WeibullDistributionBackend() {
    delete _backend;
  }
};


BasicWeibullDistribution::BasicWeibullDistribution(double k, double lambda)
{
  _dis_parameters["k"] = k; //shape
  _dis_parameters["lambda"] = lambda; //scale

  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = 0.0;
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = std::numeric_limits<double>::max( );
  }

  if ((lambda<0) || (k<0))
    throwError("ERROR: incorrect value of k or lambda for weibull distribution");

  _backend = new WeibullDistributionBackend(k, lambda);
}

BasicWeibullDistribution::~BasicWeibullDistribution()
{
  delete _backend;
}


double
BasicWeibullDistribution::untrCdf(double x){
  if(x >= 0) {
    return _backend->cdf(x);
  } else {
    return 0.0;
  }
}

/*
 * CLASS GAMMA DISTRIBUTION
 */


class GammaDistributionBackend : public DistributionBackendTemplate<boost::math::gamma_distribution<> > {
public:
  GammaDistributionBackend(double shape, double scale) {
    _backend = new boost::math::gamma_distribution<>(shape, scale);
  }
  ~GammaDistributionBackend() {
    delete _backend;
  }
};


BasicGammaDistribution::BasicGammaDistribution(double k, double theta, double low)
{
  _dis_parameters["k"] = k; //shape
  _dis_parameters["theta"] = theta; //scale
  _dis_parameters["low"] = low; //low value shift. 0.0 would be a regular gamma
  // distribution

  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = low;
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = std::numeric_limits<double>::max( );
  }


  if ((theta<0) || (k<0))
    throwError("ERROR: incorrect value of k or theta for gamma distribution");

  _backend = new GammaDistributionBackend(k, theta);
}

BasicGammaDistribution::~BasicGammaDistribution()
{
  delete _backend;
}


double
BasicGammaDistribution::untrCdf(double x){
  double low = _dis_parameters.find("low") ->second;
  if(x > 1.0e100) {
    return 1.0;
  } else if(x >= low) {
    return _backend->cdf(x - low);
  } else  {
    return 0.0;
  }
}


double
BasicGammaDistribution::untrPdf(double x){
  double low = _dis_parameters.find("low") ->second;
  return BasicTruncatedDistribution::untrPdf(x - low);
}

double
BasicGammaDistribution::untrInverseCdf(double x){
  double low = _dis_parameters.find("low") ->second;
  return BasicTruncatedDistribution::untrInverseCdf(x) + low;
}

/*
 * CLASS BETA DISTRIBUTION
 */


class BetaDistributionBackend : public DistributionBackendTemplate<boost::math::beta_distribution<> > {
public:
  BetaDistributionBackend(double alpha, double beta) {
    _backend = new boost::math::beta_distribution<>(alpha, beta);
  }
  ~BetaDistributionBackend() {
    delete _backend;
  }
};


BasicBetaDistribution::BasicBetaDistribution(double alpha, double beta, double scale)
{
  _dis_parameters["alpha"] = alpha;
  _dis_parameters["beta"] = beta;
  _dis_parameters["scale"] = scale;

  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = 0.0;
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = std::numeric_limits<double>::max( );
  }

  if ((alpha<0) || (beta<0))
    throwError("ERROR: incorrect value of alpha or beta for beta distribution");

  _backend = new BetaDistributionBackend(alpha, beta);
}

BasicBetaDistribution::~BasicBetaDistribution()
{
  delete _backend;
}

double
BasicBetaDistribution::untrCdf(double x){
  if(x >= 0 and x <= 1) {
    return _backend->cdf(x);
  } else if(x < 0){
    return 0.0;
  } else {
    return 1.0;
  }
}

double
BasicBetaDistribution::Pdf(double x){
  double scale = _dis_parameters.find("scale") ->second;
  return BasicTruncatedDistribution::Pdf(x/scale)/scale;
}

double
BasicBetaDistribution::Cdf(double x){
  double scale = _dis_parameters.find("scale") ->second;
  return BasicTruncatedDistribution::Cdf(x/scale);
}

double
BasicBetaDistribution::InverseCdf(double x){
  double scale = _dis_parameters.find("scale") ->second;
  return BasicTruncatedDistribution::InverseCdf(x)*scale;
}

/*
 * CLASS POISSON DISTRIBUTION
 */


class PoissonDistributionBackend : public DistributionBackendTemplate<boost::math::poisson_distribution<> > {
public:
  PoissonDistributionBackend(double mu) {
    _backend = new boost::math::poisson_distribution<>(mu);
  }
  ~PoissonDistributionBackend() {
    delete _backend;
  }
};


BasicPoissonDistribution::BasicPoissonDistribution(double mu)
{
  _dis_parameters["mu"] = mu;

  if(not hasParameter("truncation")) {
    _dis_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dis_parameters["xMin"] = -std::numeric_limits<double>::max( );
  }
  if(not hasParameter("xMax")) {
    _dis_parameters["xMax"] = std::numeric_limits<double>::max( );
  }

  if (mu<0)
    throwError("ERROR: incorrect value of mu for poisson distribution");

  _backend = new PoissonDistributionBackend(mu);
}

BasicPoissonDistribution::~BasicPoissonDistribution()
{
  delete _backend;
}

double
BasicPoissonDistribution::untrCdf(double x){
  if(x >= 0) {
    return _backend->cdf(x);
  } else {
    return 0.0;
  }
}

double
BasicPoissonDistribution::Pdf(double x){
   double xMin = _dis_parameters.find("xMin") ->second;
   double xMax = _dis_parameters.find("xMax") ->second;

   double value;

   if (_dis_parameters.find("truncation") ->second == 1)
          if (x<xMin)
                  value=0;
          else if (x>xMax)
                  value=0;
          else
                  value = 1/(untrCdf(xMax) - untrCdf(xMin)) * untrPdf(x);
   else
      value=-1;

   return value;
}

double
BasicPoissonDistribution::Cdf(double x){
   double xMin = _dis_parameters.find("xMin") ->second;
   double xMax = _dis_parameters.find("xMax") ->second;

   double value;

   if (_dis_parameters.find("truncation") ->second == 1)
          if (x<xMin)
                  value=0;
          else if (x>xMax)
                  value=1;
          else
                  value = 1/(untrCdf(xMax) - untrCdf(xMin)) * (untrCdf(x) - untrCdf(xMin));
   else
      value=-1;

   return value;
}

double
BasicPoissonDistribution::InverseCdf(double x){
   double value;
   double xMin = _dis_parameters.find("xMin") ->second;
   double xMax = _dis_parameters.find("xMax") ->second;

   if(x == 1.0) {
     return xMax;
   }
   if (_dis_parameters.find("truncation") ->second == 1){
     double temp = untrCdf(xMin) + x * (untrCdf(xMax)-untrCdf(xMin));
     value=untrInverseCdf(temp);
   } else {
      value=-1;
   }
   return value;
}

/*
 * CLASS BINOMIAL DISTRIBUTION
 */


class BinomialDistributionBackend : public DistributionBackendTemplate<boost::math::binomial_distribution<> > {
public:
  BinomialDistributionBackend(double n, double p) {
    _backend = new boost::math::binomial_distribution<>(n, p);
  }
  ~BinomialDistributionBackend() {
    delete _backend;
  }
};


BasicBinomialDistribution::BasicBinomialDistribution(double n, double p)
{
  _dis_parameters["n"] = n;
  _dis_parameters["p"] = p;

  if (n<0 or p<0)
    throwError("ERROR: incorrect value of n or p for binomial distribution");

  _backend = new BinomialDistributionBackend(n, p);
}

BasicBinomialDistribution::~BasicBinomialDistribution()
{
  delete _backend;
}

double
BasicBinomialDistribution::untrCdf(double x){
  if(x >= 0) {
    return _backend->cdf(x);
  } else {
    return 0.0;
  }
}

/*
 * CLASS BERNOULLI DISTRIBUTION
 */


class BernoulliDistributionBackend : public DistributionBackendTemplate<boost::math::bernoulli_distribution<> > {
public:
  BernoulliDistributionBackend(double p) {
    _backend = new boost::math::bernoulli_distribution<>(p);
  }
  ~BernoulliDistributionBackend() {
    delete _backend;
  }
};


BasicBernoulliDistribution::BasicBernoulliDistribution(double p)
{
  _dis_parameters["p"] = p;

  if (p<0)
    throwError("ERROR: incorrect value of p for bernoulli distribution");

  _backend = new BernoulliDistributionBackend(p);
}

BasicBernoulliDistribution::~BasicBernoulliDistribution()
{
  delete _backend;
}

BasicConstantDistribution::BasicConstantDistribution(double value){
  _value = value;
}
BasicConstantDistribution::~BasicConstantDistribution(){}
double  BasicConstantDistribution::Pdf(double x){
  return untrPdf(x);
}
double  BasicConstantDistribution::Cdf(double x){
  return untrCdf(x);
}
double  BasicConstantDistribution::InverseCdf(double x){
  return untrInverseCdf(x);
}

double BasicConstantDistribution::untrPdf(double x){
  if(x == _value){
    return std::numeric_limits<double>::max( )/2.0;
  } else {
    return 0.0;
  }
}

double BasicConstantDistribution::untrCdf(double x){
  if(x < _value) {
    return 0.0;
  } else if(x > _value) {
    return 1.0;
  } else {
    return 0.5;
  }
}

double BasicConstantDistribution::untrCdfComplement(double){
  throwError("Not Implemented");
  return _value;
}

double BasicConstantDistribution::untrInverseCdf(double){
  return _value;
}

double BasicConstantDistribution::untrMean(){
  return _value;
}

double BasicConstantDistribution::untrMedian(){
  return _value;
}

double BasicConstantDistribution::untrMode(){
  return _value;
}

double BasicConstantDistribution::untrHazard(double /*x*/){
  throwError("Not implemented");
  return _value;
}


/*
 * CLASS CUSTOM DISTRIBUTION
 */


// BasicCustomDistribution::BasicCustomDistribution(double x_coordinates, double y_coordinates, int fitting_type, double n_points)
// {
//    _dis_parameters["x_coordinates"] = x_coordinates;
//    _dis_parameters["y_coordinates"] = y_coordinates;
//    _dis_parameters["fitting_type"] = fitting_type;
//    _dis_parameters["n_points"] = n_points;

// }

// BasicCustomDistribution::~BasicCustomDistribution()
// {
// }

// double
// BasicCustomDistribution::Pdf(double & x){
//    double value=_interpolation.interpolation(x);

//    return value;
// }

// double
// BasicCustomDistribution::Cdf(double & ){
//   //XXX implement
//    double value=-1;

//    return value;
// }

// double
// BasicCustomDistribution::InverseCdf(double & ){
//   //XXX implement
//    double value=-1;
//    return value;
// }



//
//   // Beta pdf
//      double distribution_1D::betaPdf (double x){
//         // parameter1=alpha   >0
//         // parameter2=beta    >0
//         // 0<x<1
//
//         double value;
//
//         /*if ((x > 0)&&(x < 1)&&(_parameter1 > 0)&&(_parameter2 > 0))
//              value = 1/betaFunc(_parameter1,_parameter2)*pow(x,_parameter1-1)*pow(1-x,_parameter2-1);
//              else */
//            value=-1;
//
//         return value;
//      }
//
//      double distribution_1D::betaCdf (double x){
//         // parameter1=alpha   >0
//         // parameter2=beta    >0
//         // 0<x<1
//
//         double value;
//
//         /*if ((x > 0)&&(x < 1)&&(_parameter1 > 0)&&(_parameter2 > 0))
//            value = betaInc(_parameter1,_parameter2 ,x);
//                else */
//            value=-1;
//
//         return value;
//      }

//
//   // Gamma pdf
//      double distribution_1D::gammaPdf(double x){
//         // parameter1= k   >0
//         // parameter2= theta     >0
//         // x>=0
//
//         double value;
//
//         /* if ((x >= 0)&&(_parameter1 > 0)&&(_parameter2 > 0))
//            value=1/gammaFunc(_parameter1)/pow(_parameter2,_parameter1)*pow(x,_parameter1-1)*exp(-x/_parameter2);
//                else */
//            value=1;
//
//         return value;
//      }
//
//      double distribution_1D::gammaCdf(double x){
//         // parameter1=alpha, k   >0
//         // parameter2=beta, theta     >0
//         // x>=0
//
//         double value;
//
//         /* if ((x >= 0)&&(_parameter1 > 0)&&(_parameter2 > 0))
//              value= gammp(_parameter1,x/_parameter2);
//              else */
//            value=1;
//
//         return value;
//      }


//      double distribution_1D::gammaRandNumberGenerator(){
//          double value=-1;//gammaRNG(_parameter1,_parameter2);
//         return value;
//      }
//
//      double distribution_1D::betaRandNumberGenerator(){
//          double value=-1;//betaRNG(_parameter1,_parameter2);
//         return value;
//      }
//
//      double distribution_1D::triangularRandNumberGenerator(){
//         double value;
//         double RNG = rand()/double(RAND_MAX);
//         double referenceValue=(_parameter1-_xMin)/(_xMax-_xMin);
//
//         if (RNG<referenceValue)
//            value= _xMin+sqrt(RNG*(_parameter1-_xMin)*(_xMax-_xMin));
//         else
//            value=_xMax-sqrt((1-RNG)*(_xMax-_parameter1)*(_xMax-_xMin));
//         return value;
//      }
