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
#include <cmath>               // needed to use erfc error function
#include <string>
#include "dynamicArray.h"
#include <ctime>
#include <cstdlib>
//#include "InterpolationFunctions.h"
#include <string>
#include <limits>
#include <iso646.h>
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
#include <boost/math/distributions/laplace.hpp>
#include <boost/math/distributions/geometric.hpp>

#define _USE_MATH_DEFINES   // needed in order to use M_PI = 3.14159

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

class DistributionBackend {
public:
  virtual double pdf(double x) = 0;
  virtual double cdf(double x) = 0;
  virtual double cdfComplement(double x) = 0;
  virtual double quantile(double x) = 0;
  virtual double mean() = 0;
  virtual double standard_deviation() = 0;
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

BasicTruncatedDistribution::BasicTruncatedDistribution(double x_min, double x_max)
{
  if(not hasParameter("truncation"))
  {
    _dist_parameters["truncation"] = 1.0;
  }
    _dist_parameters["xMin"] = x_min;
    _dist_parameters["xMax"] = x_max;
}

double
BasicTruncatedDistribution::pdf(double x){
  double value;
  double x_min = _dist_parameters.find("xMin") ->second;
  double x_max = _dist_parameters.find("xMax") ->second;

  if (_dist_parameters.find("truncation") ->second == 1) {
    if ((x<x_min)||(x>x_max)) {
      value=0;
    } else {
      value = 1/(untrCdf(x_max) - untrCdf(x_min)) * untrPdf(x);
    }
  } else {
    value=-1;
  }

  return value;
}

double
BasicTruncatedDistribution::cdf(double x){
  double value;
  double x_min = _dist_parameters.find("xMin") ->second;
  double x_max = _dist_parameters.find("xMax") ->second;

  if (_dist_parameters.find("truncation") ->second == 1) {
    if (x<x_min) {
      value=0;
    } else if (x>x_max) {
      value=1;
    } else{
      value = 1/(untrCdf(x_max) - untrCdf(x_min)) * (untrCdf(x)- untrCdf(x_min));
    }
  } else {
    value=-1;
  }

  return value;
}

double
BasicTruncatedDistribution::inverseCdf(double x){
  double value;
  double x_min = _dist_parameters.find("xMin") ->second;
  double x_max = _dist_parameters.find("xMax") ->second;

  if(x == 0.0) {
    //Using == in floats is generally a bad idea, but
    // 0.0 can be represented exactly.
    //In this case, return the minimum value
    return x_min;
  }
  if(x == 1.0) {
    //Using == in floats is generally a bad idea, but
    // 1.0 can be represented exactly.
    //In this case, return the maximum value
    return x_max;
  }
  if (_dist_parameters.find("truncation") ->second == 1){
    double temp=untrCdf(x_min)+x*(untrCdf(x_max)-untrCdf(x_min));
    value=untrInverseCdf(temp);
  } else {
    throwError("A valid solution for inverseCdf was not found!");
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

/**
   Calculates the untruncated standard deviation
   \return the standard deviation
*/
double BasicTruncatedDistribution::untrStdDev() {
  return _backend->standard_deviation();
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

/**
   Calculates the untruncated standard deviation
   \return the standard deviation
*/
double BasicDiscreteDistribution::untrStdDev() {
  return _backend->standard_deviation();
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

double BasicDiscreteDistribution::pdf(double x) {
  return untrPdf(x);
}

double BasicDiscreteDistribution::cdf(double x) {
  return untrCdf(x);
}

double BasicDiscreteDistribution::inverseCdf(double x) {
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
  double standard_deviation() { return boost::math::standard_deviation(*_backend); };
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
  UniformDistributionBackend(double x_min, double x_max) {
    _backend = new boost::math::uniform(x_min,x_max);
  }
  ~UniformDistributionBackend() {
    delete _backend;
  }
};


BasicUniformDistribution::BasicUniformDistribution(double x_min, double x_max)
{
  _dist_parameters["xMin"] = x_min;
  _dist_parameters["xMax"] = x_max;
  _backend = new UniformDistributionBackend(x_min, x_max);

  if (x_min>x_max)
    throwError("ERROR: bounds for uniform distribution are incorrect");
}

BasicUniformDistribution::~BasicUniformDistribution()
{
  delete _backend;
}

double
BasicUniformDistribution::pdf(double x){
  return untrPdf(x);
}

double
BasicUniformDistribution::cdf(double x){
  return untrCdf(x);
}

double
BasicUniformDistribution::inverseCdf(double x){
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
  _dist_parameters["mu"] = mu; //mean
  _dist_parameters["sigma"] = sigma; //sd
  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = -std::numeric_limits<double>::max( );
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = std::numeric_limits<double>::max( );
  }
  //std::cout << "mu " << mu << " sigma " << sigma
  //          << " truncation " << _dist_parameters["truncation"]
  //          << " xMin " << _dist_parameters["xMin"]
  //          << " xMax " << _dist_parameters["xMax"] << std::endl;
  _backend = new NormalDistributionBackend(mu, sigma);
}

BasicNormalDistribution::BasicNormalDistribution(double mu, double sigma, double x_min, double x_max):
  BasicTruncatedDistribution(x_min,x_max)
{
  _dist_parameters["mu"] = mu; //mean
  _dist_parameters["sigma"] = sigma; //sd
  //if(not hasParameter("truncation")) {
  //  _dist_parameters["truncation"] = 1.0;
  //}
  //_dist_parameters["xMin"] = x_min;
  //_dist_parameters["xMax"] = x_max;
  //std::cout << "mu " << mu << " sigma " << sigma
  //          << " truncation " << _dist_parameters["truncation"]
  //          << " xMin " << _dist_parameters["xMin"]
  //          << " xMax " << _dist_parameters["xMax"] << std::endl;
  _backend = new NormalDistributionBackend(mu, sigma);

}


BasicNormalDistribution::~BasicNormalDistribution(){
  delete _backend;
}


double
BasicNormalDistribution::inverseCdf(double x){
  return BasicTruncatedDistribution::inverseCdf(x);
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


BasicLogNormalDistribution::BasicLogNormalDistribution(double mu, double sigma, double low)
{
  _dist_parameters["mu"] = mu;
  _dist_parameters["sigma"] = sigma;
  _dist_parameters["low"] = low;

  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = low;
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = std::numeric_limits<double>::max( );
  }

  _backend = new LogNormalDistributionBackend(mu, sigma);

}

BasicLogNormalDistribution::BasicLogNormalDistribution(double mu, double sigma, double x_min, double x_max, double low):
  BasicTruncatedDistribution(x_min,x_max)
{
  _dist_parameters["mu"] = mu;
  _dist_parameters["sigma"] = sigma;
  _dist_parameters["low"] = low;

  _backend = new LogNormalDistributionBackend(mu, sigma);

}


BasicLogNormalDistribution::~BasicLogNormalDistribution()
{
  delete _backend;
}

double
BasicLogNormalDistribution::untrPdf(double x){
  double low = _dist_parameters.find("low") ->second;
  if(x <= low) {
    return 0.0;
  } else {
    return _backend->pdf(x-low);
  }
}

double
BasicLogNormalDistribution::untrCdf(double x){
  double low = _dist_parameters.find("low") ->second;
  if(x <= low) {
    return 0.0;
  } else {
    return _backend->cdf(x-low);
  }
}


double
BasicLogNormalDistribution::inverseCdf(double x){
  double low = _dist_parameters.find("low") ->second;
  return BasicTruncatedDistribution::inverseCdf(x)+low;
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
  _dist_parameters["location"] = location;
  _dist_parameters["scale"] = scale;

  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = -std::numeric_limits<double>::max( );
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = std::numeric_limits<double>::max( );
  }

  _backend = new LogisticDistributionBackend(location, scale);
}

BasicLogisticDistribution::BasicLogisticDistribution(double location, double scale, double x_min, double x_max):
  BasicTruncatedDistribution(x_min,x_max)
{
    _dist_parameters["location"] = location;
    _dist_parameters["scale"] = scale;

    _backend = new LogisticDistributionBackend(location, scale);
}

BasicLogisticDistribution::~BasicLogisticDistribution()
{
  delete _backend;
}

/*
 * CLASS LAPLACE DISTRIBUTION
 */
class LaplaceDistributionBackend : public DistributionBackendTemplate<boost::math::laplace_distribution<> > {
public:
  LaplaceDistributionBackend(double location, double scale) {
    _backend = new boost::math::laplace_distribution<>(location, scale);
  }
  ~LaplaceDistributionBackend() {
    delete _backend;
  }
};

BasicLaplaceDistribution::BasicLaplaceDistribution(double location, double scale, double x_min, double x_max):
    BasicTruncatedDistribution(x_min,x_max)
{
  _dist_parameters["location"] = location;
  _dist_parameters["scale"] = scale;

  _backend = new LaplaceDistributionBackend(location, scale);
}

BasicLaplaceDistribution::~BasicLaplaceDistribution()
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


BasicTriangularDistribution::BasicTriangularDistribution(double x_peak, double lower_bound, double upper_bound)
{
  _dist_parameters["xPeak"] = x_peak;
  _dist_parameters["lowerBound"] = lower_bound;
  _dist_parameters["upperBound"] = upper_bound;

  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = lower_bound;
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = upper_bound;
  }


  if (upper_bound < lower_bound)
    throwError("ERROR: bounds for triangular distribution are incorrect");
  if (upper_bound < _dist_parameters.find("xMin") ->second)
    throwError("ERROR: bounds and LB/UB are inconsistent for triangular distribution");
  if (lower_bound > _dist_parameters.find("xMax") ->second)
    throwError("ERROR: bounds and LB/UB are inconsistent for triangular distribution");
  _backend = new TriangularDistributionBackend(lower_bound, x_peak, upper_bound);

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


BasicExponentialDistribution::BasicExponentialDistribution(double lambda, double low)
{
  _dist_parameters["lambda"] = lambda;
  _dist_parameters["low"] = low;

  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = low;
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = std::numeric_limits<double>::max( );
  }


  if (lambda<0)
    throwError("ERROR: incorrect value of lambda for exponential distribution");

  _backend = new ExponentialDistributionBackend(lambda);
}


BasicExponentialDistribution::BasicExponentialDistribution(double lambda, double x_min, double x_max, double low):
  BasicTruncatedDistribution(x_min,x_max)
{
    _dist_parameters["lambda"] = lambda;
    _dist_parameters["low"] = low;
    if (lambda<0)
    throwError("ERROR: incorrect value of lambda for exponential distribution");
    _backend = new ExponentialDistributionBackend(lambda);
}


BasicExponentialDistribution::~BasicExponentialDistribution()
{
  delete _backend;
}



double
BasicExponentialDistribution::untrPdf(double x){
  double low = _dist_parameters.find("low") ->second;
  if(x >= low) {
    return _backend->pdf(x-low);
  } else {
    return 0.0;
  }
}

double
BasicExponentialDistribution::untrCdf(double x){
  double low = _dist_parameters.find("low") ->second;
  if(x >= low) {
    return _backend->cdf(x-low);
  } else {
    return 0.0;
  }
}

double
BasicExponentialDistribution::cdf(double x){
  return BasicTruncatedDistribution::cdf(x);
  //double low = _dist_parameters.find("low") ->second;
}

double
BasicExponentialDistribution::inverseCdf(double x){
  double low = _dist_parameters.find("low") ->second;
  return BasicTruncatedDistribution::inverseCdf(x) + low;
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


BasicWeibullDistribution::BasicWeibullDistribution(double k, double lambda, double low)
{
  _dist_parameters["k"] = k; //shape
  _dist_parameters["lambda"] = lambda; //scale
  _dist_parameters["low"] = low; //scale

  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = low;
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = std::numeric_limits<double>::max( );
  }

  if ((lambda<0) || (k<0))
    throwError("ERROR: incorrect value of k or lambda for weibull distribution");

  _backend = new WeibullDistributionBackend(k, lambda);
}

BasicWeibullDistribution::BasicWeibullDistribution(double k, double lambda, double x_min, double x_max, double low):
  BasicTruncatedDistribution(x_min,x_max)
{
    _dist_parameters["k"] = k; //shape
    _dist_parameters["lambda"] = lambda; //scale
    _dist_parameters["low"] = low; //scale

    if ((lambda<0) || (k<0))
    throwError("ERROR: incorrect value of k or lambda for weibull distribution");
    _backend = new WeibullDistributionBackend(k, lambda);
}

BasicWeibullDistribution::~BasicWeibullDistribution()
{
  delete _backend;
}


double
BasicWeibullDistribution::untrPdf(double x){
  double low = _dist_parameters.find("low") ->second;
  if(x >= low) {
    return _backend->pdf(x-low);
  } else {
    return 0.0;
  }
}

double
BasicWeibullDistribution::untrCdf(double x){
  double low = _dist_parameters.find("low") ->second;
  if(x >= low) {
    return _backend->cdf(x-low);
  } else {
    return 0.0;
  }
}

double
BasicWeibullDistribution::inverseCdf(double x){
  double low = _dist_parameters.find("low") ->second;
  return BasicTruncatedDistribution::inverseCdf(x) + low;
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
  _dist_parameters["k"] = k; //shape
  _dist_parameters["theta"] = theta; //scale
  _dist_parameters["low"] = low; //low value shift. 0.0 would be a regular gamma
  // distribution

  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = low;
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = std::numeric_limits<double>::max( );
  }


  if ((theta<0) || (k<0))
    throwError("ERROR: incorrect value of k or theta for gamma distribution");

  _backend = new GammaDistributionBackend(k, theta);
}

BasicGammaDistribution::BasicGammaDistribution(double k, double theta, double low, double x_min, double x_max):
  BasicTruncatedDistribution(x_min,x_max)
{
    _dist_parameters["k"] = k; //shape
    _dist_parameters["theta"] = theta; //scale
    _dist_parameters["low"] = low; //low value shift. 0.0 would be a regular gamma
    // distribution

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
  double low = _dist_parameters.find("low") ->second;
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
  double low = _dist_parameters.find("low") ->second;
  return BasicTruncatedDistribution::untrPdf(x - low);
}

double
BasicGammaDistribution::untrInverseCdf(double x){
  double low = _dist_parameters.find("low") ->second;
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


BasicBetaDistribution::BasicBetaDistribution(double alpha, double beta, double scale, double low)
{
  _dist_parameters["alpha"] = alpha;
  _dist_parameters["beta" ] = beta;
  _dist_parameters["scale"] = scale;
  _dist_parameters["low"  ] = low;

  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = low;
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = low+scale;
  }

  if ((alpha<0) || (beta<0))
    throwError("ERROR: incorrect value of alpha or beta for beta distribution");

  _backend = new BetaDistributionBackend(alpha, beta);
}

BasicBetaDistribution::BasicBetaDistribution(double alpha, double beta, double scale, double x_min, double x_max, double low):
  BasicTruncatedDistribution(x_min,x_max)
{
    _dist_parameters["alpha"] = alpha;
    _dist_parameters["beta" ] = beta;
    _dist_parameters["scale"] = scale;
    _dist_parameters["low"  ] = low;

    if ((alpha<0) || (beta<0))
    throwError("ERROR: incorrect value of alpha or beta for beta distribution");


    _backend = new BetaDistributionBackend(alpha, beta);
}

BasicBetaDistribution::~BasicBetaDistribution()
{
  delete _backend;
}

double
BasicBetaDistribution::untrPdf(double x){
  double scale = _dist_parameters.find("scale") ->second;
  double low   = _dist_parameters.find("low"  ) ->second;
  return _backend->pdf( (x-low)/scale);
}

double
BasicBetaDistribution::untrCdf(double x){
  double scale = _dist_parameters.find("scale") ->second;
  double low   = _dist_parameters.find("low"  ) ->second;
  if(x >= low and x <= low+scale) {
    return _backend->cdf( (x-low)/scale );
  } else if(x < low){
    return 0.0;
  } else {
    return 1.0;
  }
}

double
BasicBetaDistribution::pdf(double x){
  double scale   = _dist_parameters.find("scale"  ) ->second;
  return BasicTruncatedDistribution::pdf( x )/scale;// scaling happens in untrPdf
}

double
BasicBetaDistribution::cdf(double x){
  //double scale = _dist_parameters.find("scale") ->second;
  //double low   = _dist_parameters.find("low"  ) ->second;
  return BasicTruncatedDistribution::cdf( x );// -low)/scale ); scaling happens in untrCdf
}

double
BasicBetaDistribution::inverseCdf(double x){
  double scale = _dist_parameters.find("scale") ->second;
  double low   = _dist_parameters.find("low"  ) ->second;
  return BasicTruncatedDistribution::inverseCdf( x )*scale+low;
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
  _dist_parameters["mu"] = mu;

  if(not hasParameter("truncation")) {
    _dist_parameters["truncation"] = 1.0;
  }
  if(not hasParameter("xMin")) {
    _dist_parameters["xMin"] = -std::numeric_limits<double>::max( );
  }
  if(not hasParameter("xMax")) {
    _dist_parameters["xMax"] = std::numeric_limits<double>::max( );
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
BasicPoissonDistribution::pdf(double x){
   double x_min = _dist_parameters.find("xMin") ->second;
   double x_max = _dist_parameters.find("xMax") ->second;

   double value;

   if (_dist_parameters.find("truncation") ->second == 1)
          if (x<x_min)
                  value=0;
          else if (x>x_max)
                  value=0;
          else
                  value = 1/(untrCdf(x_max) - untrCdf(x_min)) * untrPdf(x);
   else
      value=-1;

   return value;
}

double
BasicPoissonDistribution::cdf(double x){
   double x_min = _dist_parameters.find("xMin") ->second;
   double x_max = _dist_parameters.find("xMax") ->second;

   double value;

   if (_dist_parameters.find("truncation") ->second == 1)
          if (x<x_min)
                  value=0;
          else if (x>x_max)
                  value=1;
          else
                  value = 1/(untrCdf(x_max) - untrCdf(x_min)) * (untrCdf(x) - untrCdf(x_min));
   else
      value=-1;

   return value;
}

double
BasicPoissonDistribution::inverseCdf(double x){
   double value;
   double x_min = _dist_parameters.find("xMin") ->second;
   double x_max = _dist_parameters.find("xMax") ->second;

   if(x == 1.0) {
     return x_max;
   }
   if (_dist_parameters.find("truncation") ->second == 1){
     double temp = untrCdf(x_min) + x * (untrCdf(x_max)-untrCdf(x_min));
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
  _dist_parameters["n"] = n;
  _dist_parameters["p"] = p;

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
  _dist_parameters["p"] = p;

  if (p<0)
    throwError("ERROR: incorrect value of p for bernoulli distribution");

  _backend = new BernoulliDistributionBackend(p);
}

BasicBernoulliDistribution::~BasicBernoulliDistribution()
{
  delete _backend;
}

/*
 * CLASS GEOMETRIC DISTRIBUTION
 */

class GeometricDistributionBackend : public DistributionBackendTemplate<boost::math::geometric_distribution<> > {
public:
  GeometricDistributionBackend(double p) {
    _backend = new boost::math::geometric_distribution<>(p);
  }
  ~GeometricDistributionBackend() {
    delete _backend;
  }
};

BasicGeometricDistribution::BasicGeometricDistribution(double p)
{
  _dist_parameters["p"] = p;

  if (p<0)
    throwError("ERROR: incorrect value of p for geometric distribution");

  _backend = new GeometricDistributionBackend(p);
}

BasicGeometricDistribution::~BasicGeometricDistribution()
{
  delete _backend;
}

/*
 * CLASS CONSTANT DISTRIBUTION
 */

BasicConstantDistribution::BasicConstantDistribution(double value){
  _value = value;
}
BasicConstantDistribution::~BasicConstantDistribution(){}
double  BasicConstantDistribution::pdf(double x){
  return untrPdf(x);
}
double  BasicConstantDistribution::cdf(double x){
  return untrCdf(x);
}
double  BasicConstantDistribution::inverseCdf(double x){
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

/**
   Calculates the untruncated standard deviation
   \return the standard deviation
*/
double BasicConstantDistribution::untrStdDev(){
  return 0.0;
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
//    _dist_parameters["x_coordinates"] = x_coordinates;
//    _dist_parameters["y_coordinates"] = y_coordinates;
//    _dist_parameters["fitting_type"] = fitting_type;
//    _dist_parameters["n_points"] = n_points;

// }

// BasicCustomDistribution::~BasicCustomDistribution()
// {
// }

// double
// BasicCustomDistribution::pdf(double & x){
//    double value=_interpolation.interpolation(x);

//    return value;
// }

// double
// BasicCustomDistribution::cdf(double & ){
//   //XXX implement
//    double value=-1;

//    return value;
// }

// double
// BasicCustomDistribution::inverseCdf(double & ){
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
//         double referenceValue=(_parameter1-_x_min)/(_x_max-_x_min);
//
//         if (RNG<referenceValue)
//            value= _x_min+sqrt(RNG*(_parameter1-_x_min)*(_x_max-_x_min));
//         else
//            value=_x_max-sqrt((1-RNG)*(_x_max-_parameter1)*(_x_max-_x_min));
//         return value;
//      }
