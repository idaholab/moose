/*
 * distribution_base_ND.h
 *
 *  Created on: Feb 6, 2014
 *      Author: alfoa
 *
 */

#ifndef DISTRIBUTION_BASE_ND_H_
#define DISTRIBUTION_BASE_ND_H_
#include <map>
#include <string>
#include <vector>
#include "ND_Interpolation_Functions.h"
//#include "distribution_min.h"
#include <iostream>

enum PbFunctionType{PDF,CDF};

class distributionND;

class BasicDistributionND
{
public:
   BasicDistributionND();
   virtual ~BasicDistributionND();
   double  getVariable(const std::string & variableName);                       ///< getVariable from mapping
   void updateVariable(const std::string & variableName, double & newValue);
   virtual double  Pdf(std::vector<double> x) = 0;                           ///< Pdf function at coordinate x
   virtual double  Cdf(std::vector<double> x) = 0;                     ///< Cdf function at coordinate x
   virtual std::vector<double> InverseCdf(double min, double max) = 0;

   std::string & getType();

protected:
   std::string                   _type;                              ///< Distribution type
   std::string                   _data_filename;
   PbFunctionType                _function_type;
   std::map <std::string,double> _dis_parameters;
   bool                          _checkStatus;

};

class BasicMultiDimensionalInverseWeight: public virtual BasicDistributionND
{
public:
  BasicMultiDimensionalInverseWeight(std::string data_filename,double p):  _interpolator(data_filename,p){};
  BasicMultiDimensionalInverseWeight(double p):  _interpolator(inverseDistanceWeigthing(p)){};
  virtual ~BasicMultiDimensionalInverseWeight(){};
  double  Pdf(std::vector<double> x) {return _interpolator.interpolateAt(x);};
  double  Cdf(std::vector<double> x) {return _interpolator.interpolateAt(x);};
  std::vector<double> InverseCdf(double min, double max) {return _interpolator.NDinverseFunction(min, max);};
protected:
  inverseDistanceWeigthing  _interpolator;
};

class BasicMultivariateNormal: public virtual BasicDistributionND
{
public:
  BasicMultivariateNormal(std::string data_filename, std::vector<double> mu,std::vector<double> sigma);
  BasicMultivariateNormal(std::vector<std::vector<double> > covMatrix, std::vector<double> mu, std::vector<double> sigma);
  virtual ~BasicMultivariateNormal();
  double  Pdf(std::vector<double> x);
  double  Cdf(std::vector<double> x);
  std::vector<double> InverseCdf(double /*min*/, double /*max*/) {return std::vector<double>(2,-1.0);};
private:
  std::vector<double> _mu;
  std::vector<double> _sigma;
  std::vector<std::vector<double> > _covMatrix;
  std::vector<std::vector<double> > _inverseCovMatrix;
  double _determinantCovMatrix;
};


class BasicMultiDimensionalScatteredMS: public virtual BasicDistributionND
{
public:
  BasicMultiDimensionalScatteredMS(std::string data_filename,double p,int precision): _interpolator(data_filename,p,precision){};
  BasicMultiDimensionalScatteredMS(double p,int precision): _interpolator(p,precision){};
  virtual ~BasicMultiDimensionalScatteredMS(){};
  double  Pdf(std::vector<double> x) {return _interpolator.interpolateAt(x);};
  double  Cdf(std::vector<double> x){return _interpolator.interpolateAt(x);};
  double  InverseCdf(std::vector<double> /*x*/){return -1.0;};
  std::vector<double> InverseCdf(double /*min*/, double /*max*/) {return std::vector<double>(2,-1.0);};
protected:
  microSphere _interpolator;
};

class BasicMultiDimensionalCartesianSpline: public  virtual BasicDistributionND
{
public:
  BasicMultiDimensionalCartesianSpline(std::string data_filename,std::vector<double> alpha, std::vector<double> beta): _interpolator(data_filename, alpha, beta){};
  BasicMultiDimensionalCartesianSpline(): _interpolator(){};
  virtual ~BasicMultiDimensionalCartesianSpline(){};
  double  Pdf(std::vector<double> x) {return _interpolator.interpolateAt(x);};
  double  Cdf(std::vector<double> x){return _interpolator.interpolateAt(x);};
  double  InverseCdf(std::vector<double> /*x*/){return -1.0;};
  std::vector<double> InverseCdf(double /*min*/, double /*max*/) {return std::vector<double>(2,-1.0);};
protected:
  NDspline _interpolator;
};



double DistributionPdf(BasicDistributionND & dist,std::vector<double> & x);
double DistributionCdf(BasicDistributionND & dist,std::vector<double> & x);
std::string getDistributionType(BasicDistributionND & dist);
double getDistributionVariable(BasicDistributionND & dist,const std::string & variableName);
void DistributionUpdateVariable(BasicDistributionND & dist,const std::string & variableName, double & newValue);

#endif /* DISTRIBUTION_BASE_ND_H_ */
