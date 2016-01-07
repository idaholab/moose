/*
 * distributionNDCartesianSpline.h
 * Created by @wangc on Oct. 23, 2015
 * Extracted from  distribution_base_ND.h
 *
 */
#ifndef DISTRIBUTION_ND_CARTESIAN_SPLINE_H
#define DISTRIBUTION_ND_CARTESIAN_SPLINE_H
#include <map>
#include <string>
#include <vector>
#include "ND_Interpolation_Functions.h"
#include "distributionFunctions.h"
#include "distributionNDBase.h"
#include <stdexcept>
#include <iostream>
#include <fstream>

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

class BasicMultiDimensionalCartesianSpline: public  virtual BasicDistributionND
{
public:
  BasicMultiDimensionalCartesianSpline(const char * data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided):  _interpolator(data_filename,alpha, beta)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  };

  BasicMultiDimensionalCartesianSpline(std::string data_filename, bool CDFprovided):  _interpolator(data_filename)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  };

  BasicMultiDimensionalCartesianSpline(const char * data_filename, bool CDFprovided):  _interpolator(data_filename)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  };

  BasicMultiDimensionalCartesianSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta, bool CDFprovided):  _interpolator(discretizations, values, alpha, beta)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  };

  BasicMultiDimensionalCartesianSpline(std::string data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided): _interpolator(data_filename, alpha, beta)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  };

  void BasicMultiDimensionalCartesianSpline_init(){
    std::vector<double> alpha(_interpolator.returnDimensionality());
    std::vector<double> beta(_interpolator.returnDimensionality());

    for(int i=0; i<_interpolator.returnDimensionality(); i++){
      alpha[i] = 0.0;
      beta[i] = 0.0;
    }

    if (_CDFprovided){
     bool LBcheck = _interpolator.checkLB(0.0);
     if (LBcheck == false)
    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element below 0.0");

     bool UBcheck = _interpolator.checkUB(1.0);
     if (UBcheck == false)
    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element above 1.0");
    }

    if (_CDFprovided == false){    // PDF provided ---> create grid for CDF

      std::cout<<"Creation of CDF interpolator for cartesian spline"<< std::endl;
      std::vector< std::vector<double> > discretizations;
      _interpolator.getDiscretizations(discretizations);
      int numberofValues = 1;
      int numberOfDimensions = discretizations.size();
      std::vector<int> discretizationSizes(numberOfDimensions);

      for (int i=0; i<numberOfDimensions; i++){
        numberofValues *= discretizations.at(i).size();
        discretizationSizes.at(i) = discretizations.at(i).size();
      }

      std::vector<double> CDFvalues(numberofValues);

      for (int i=0; i<numberofValues; i++){
        std::vector<int> NDcoordinateIndex = oneDtoNDconverter(i, discretizationSizes);
        std::vector<double> NDcoordinate(numberOfDimensions);
        for (int j=0; j<numberOfDimensions; j++)
          NDcoordinate.at(j) = discretizations.at(j)[NDcoordinateIndex.at(j)];
        CDFvalues.at(i) = _interpolator.integralSpline(NDcoordinate);
        //std::cout<< NDcoordinate.at(0) << " " << NDcoordinate.at(1) << " : " << CDFvalues.at(i) << std::endl;
      }
      _CDFinterpolator = NDSpline(discretizations,CDFvalues,alpha,beta);
    }
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
    if (_CDFprovided)
      return _interpolator.NDderivative(x);
    else
      return _interpolator.interpolateAt(x);
  };

  double
  Cdf(std::vector<double> x)
  {
    double value;

    if (_CDFprovided)
      value = _interpolator.interpolateAt(x);
    else
      value = _CDFinterpolator.interpolateAt(x);

      if (value > 1.0)
    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF value calculated is above 1.0");

     return value;
  };


  std::vector<double>
  InverseCdf(double F, double g)
  {
    if (_CDFprovided == true)
      return _interpolator.NDinverseFunctionGrid(F,g);
    else{
      return _CDFinterpolator.NDinverseFunctionGrid(F,g);
    }
  };

  double inverseMarginal(double F, int dimension){
    double value=0.0;

    if ((F<1.0) and (F>0.0)){
      if (_CDFprovided){
        throwError("BasicMultiDimensionalCartesianSpline Distribution error: inverseMarginal calculation not available if CDF provided");
      }else{
        value = _interpolator.spline_cartesian_inverse_marginal(F, dimension, 0.01);
      }
    }else
      throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF value for inverse marginal distribution is above 1.0");

    return value;
  }

  int
  returnDimensionality()
  {
    return _interpolator.returnDimensionality();
  };

//  double cellIntegral(std::vector<double> center, std::vector<double> dx){
//    if (_CDFprovided){
//      return _interpolator.averageCellValue(center,dx);
//    }else{
//      return _CDFinterpolator.averageCellValue(center,dx);
//    }
//  }

  void updateRNGparameter(double tolerance, double initial_divisions){
    _tolerance = tolerance;
    _initial_divisions = (int)initial_divisions;

    if (_CDFprovided)
      _interpolator.updateRNGparameters(_tolerance,_initial_divisions);
    else{
      _interpolator.updateRNGparameters(_tolerance,_initial_divisions);
      _CDFinterpolator.updateRNGparameters(_tolerance,_initial_divisions);
    }
  };

  double Marginal(double x, int dimension){
    double value=0.0;
    if (_CDFprovided){
      throwError("BasicMultiDimensionalCartesianSpline Distribution error: Marginal calculation not available if CDF provided");
    }else{
      value = _interpolator.spline_cartesian_marginal_integration(x, dimension);
    }
    return value;
  }

  double returnUpperBound(int dimension){
	  /**
	   * this function returns the upper bound of the distribution for a particular dimension
	   */
    return _interpolator.returnUpperBound(dimension);
  }

  double returnLowerBound(int dimension){
	  /**
	   * this function returns the lower bound of the distribution for a particular dimension
	   */
    return _interpolator.returnLowerBound(dimension);
  }

protected:
  bool _CDFprovided;
  NDSpline _interpolator;
  NDSpline _CDFinterpolator;
};

#endif /* DISTRIBUTION_ND_CARTESIAN_SPLINE_H */
