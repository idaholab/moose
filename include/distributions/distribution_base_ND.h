#ifndef DISTRIBUTION_BASE_ND_H
#define DISTRIBUTION_BASE_ND_H
#include <map>
#include <string>
#include <vector>
#include "ND_Interpolation_Functions.h"
#include "distributionFunctions.h"
#include <stdexcept>
//#include "distribution_min.h"
#include <iostream>

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

enum EPbFunctionType{PDF,CDF};

class distributionND;

class BasicDistributionND
{
public:

   BasicDistributionND();
   virtual ~BasicDistributionND();
   double  getVariable(const std::string & variableName);                       ///< getVariable from mapping
   void updateVariable(const std::string & variableName, double & newValue);
   virtual double  Pdf(std::vector<double> x) = 0;                              ///< Pdf function at coordinate x
   virtual double  Cdf(std::vector<double> x) = 0;                              ///< Cdf function at coordinate x
   //virtual std::vector<double> InverseCdf(double min, double max) = 0;
   virtual std::vector<double> InverseCdf(double F, double tolerance, int initial_divisions=2) = 0;

   std::string & getType();

protected:
   std::string _type; ///< Distribution type
   std::string _data_filename;
   EPbFunctionType _function_type;
   std::map <std::string,double> _dis_parameters;
   bool _checkStatus;

};

class BasicMultiDimensionalInverseWeight: public virtual BasicDistributionND
{
public:
  BasicMultiDimensionalInverseWeight(const char * data_filename,double p):  _interpolator(data_filename,p)
  {
  };

  BasicMultiDimensionalInverseWeight(std::string data_filename,double p):  _interpolator(data_filename,p)
  {
   bool LBcheck = _interpolator.checkLB(0.0);
   if (LBcheck == false)
    throwError("BasicMultiDimensionalInverseWeight Distribution error: CDF values given as input contain element below 0.0 in file: " << data_filename);

   bool UBcheck = _interpolator.checkUB(1.0);
   if (UBcheck == false)
    throwError("BasicMultiDimensionalInverseWeight Distribution error: CDF values given as input contain element above 1.0 in file: " << data_filename);
  };
  BasicMultiDimensionalInverseWeight(double p):  _interpolator(InverseDistanceWeighting(p))
  {
  };
  virtual ~BasicMultiDimensionalInverseWeight()
  {
  };
  double
  Pdf(std::vector<double> x)
  {
    return _interpolator.interpolateAt(x);
  };
  double
  Cdf(std::vector<double> x)
  {
     double value = _interpolator.interpolateAt(x);

     if (value > 1.0)
      value=1.0;

     return value;
  };

  std::vector<double>
  InverseCdf(double F, double tolerance, int initial_divisions=2)
  {
   return _interpolator.NDinverseFunctionGrid(F, tolerance, initial_divisions);
      //return _interpolator.NDinverseFunction(min, max);
  };

protected:
  InverseDistanceWeighting  _interpolator;
};




class BasicMultivariateNormal: public virtual BasicDistributionND
{
public:
  BasicMultivariateNormal(const char * data_filename, std::vector<double> mu);
  BasicMultivariateNormal(std::string data_filename, std::vector<double> mu);
  BasicMultivariateNormal(std::vector<std::vector<double> > covMatrix, std::vector<double> mu);
  virtual ~BasicMultivariateNormal();
  double  Pdf(std::vector<double> x);
  double  Cdf(std::vector<double> x);
//  std::vector<double>
//  InverseCdf(double /*min*/, double /*max*/)
//  {
//    return std::vector<double>(2,-1.0);
//  };
  std::vector<double>
  InverseCdf(double F, double tolerance, int initial_divisions=10)
  {
   return std::vector<double>(2,-1.0);
      //return _interpolator.NDinverseFunction(min, max);
  };

  //double MVNDST(std::vector<double> a, std::vector<double> b, double alpha, double epsilon, int Nmax);
  double phi(double x);
  double phi_inv(double x);
  //double rn();
  double * cholesky(double *A, int n);
  std::vector<std::vector<double> > choleskyDecomposition(std::vector<std::vector<double> > matrix);
  void show_matrix(double *A, int n);
private:
  std::vector<double> _mu;
  std::vector<std::vector<double> > _cov_matrix;
  std::vector<std::vector<double> > _inverse_cov_matrix;
  std::vector<std::vector<double> > _cholesky_C;
  double _determinant_cov_matrix;
};




class BasicMultiDimensionalScatteredMS: public virtual BasicDistributionND
{
public:
  BasicMultiDimensionalScatteredMS(std::string data_filename,double p,int precision): _interpolator(data_filename,p,precision)
  {
  };
  BasicMultiDimensionalScatteredMS(double p,int precision): _interpolator(p,precision)
  {
  };
  virtual ~BasicMultiDimensionalScatteredMS()
  {
  };
  double
  Pdf(std::vector<double> x)
  {
    return _interpolator.interpolateAt(x);
  };
  double
  Cdf(std::vector<double> x)
  {
    return _interpolator.interpolateAt(x);
  };
  double
  InverseCdf(std::vector<double> /*x*/)
  {
    return -1.0;
  };
//  std::vector<double>
//  InverseCdf(double /*min*/, double /*max*/)
//  {
//    return std::vector<double>(2,-1.0);
//  };
  std::vector<double>
  InverseCdf(double F, double tolerance, int initial_divisions=10)
  {
   return _interpolator.NDinverseFunctionGrid(F, tolerance, initial_divisions);
      //return _interpolator.NDinverseFunction(min, max);
  };
protected:
  MicroSphere _interpolator;
};

class BasicMultiDimensionalCartesianSpline: public  virtual BasicDistributionND
{
public:
  BasicMultiDimensionalCartesianSpline(const char * data_filename,std::vector<double> alpha, std::vector<double> beta):  _interpolator(data_filename,alpha, beta)
  {
  };

  BasicMultiDimensionalCartesianSpline(std::string data_filename,std::vector<double> alpha, std::vector<double> beta): _interpolator(data_filename, alpha, beta)
  {
   bool LBcheck = _interpolator.checkLB(0.0);
   if (LBcheck == false)
    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element below 0.0 in file: " << data_filename);

   bool UBcheck = _interpolator.checkUB(1.0);
   if (UBcheck == false)
    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element above 1.0 in file: " << data_filename);
  };
  BasicMultiDimensionalCartesianSpline(): _interpolator()
  {
  };
  virtual ~BasicMultiDimensionalCartesianSpline()
  {
  };
  double
  Pdf(std::vector<double> x)
  {
    return _interpolator.interpolateAt(x);
  };
  double
  Cdf(std::vector<double> x)
  {
     double value = _interpolator.interpolateAt(x);

     if (value > 1.0)
      value=1.0;

     return value;
  };
  double
  InverseCdf(std::vector<double> /*x*/)
  {
    return -1.0;
  };
//  std::vector<double>
//  InverseCdf(double /*min*/, double /*max*/)
//  {
//    return std::vector<double>(2,-1.0);
//  };
  std::vector<double>
  InverseCdf(double F, double tolerance, int initial_divisions=10)
  {
   return _interpolator.NDinverseFunctionGrid(F, tolerance, initial_divisions);
      //return _interpolator.NDinverseFunction(min, max);
  };
protected:
  NDSpline _interpolator;
};

//class BasicMultiDimensionalLinear: public  virtual BasicDistributionND
//{
//public:
//  BasicMultiDimensionalLinear(std::string data_filename): _interpolator(data_filename)
//  {
//   bool LBcheck = _interpolator.checkLB(0.0);
//   if (LBcheck == false)
//    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element below 0.0 in file: " << data_filename);
//
//   bool UBcheck = _interpolator.checkUB(1.0);
//   if (UBcheck == false)
//    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element above 1.0 in file: " << data_filename);
//  };
//  BasicMultiDimensionalLinear(): _interpolator()
//  {
//  };
//  virtual ~BasicMultiDimensionalLinear()
//  {
//  };
//  double
//  Pdf(std::vector<double> x)
//  {
//    return _interpolator.interpolateAt(x);
//  };
//  double
//  Cdf(std::vector<double> x)
//  {
//     double value = _interpolator.interpolateAt(x);
//
//     if (value > 1.0)
//      value=1.0;
//
//     return value;
//  };
//  double
//  InverseCdf(std::vector<double> /*x*/)
//  {
//    return -1.0;
//  };
//  std::vector<double>
//  InverseCdf(double /*min*/, double /*max*/)
//  {
//    return std::vector<double>(2,-1.0);
//  };
//protected:
//  NDlinear _interpolator;
//};


#endif /* DISTRIBUTION_BASE_ND_H */
