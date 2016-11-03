
#include <map>
#include <string>
#include <vector>
#include "ND_Interpolation_Functions.h"
#include "distributionFunctions.h"
#include "distributionNDBase.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include "distributionNDCartesianSpline.h"


#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

  BasicMultiDimensionalCartesianSpline::BasicMultiDimensionalCartesianSpline(const char * data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided):  _interpolator(data_filename,alpha, beta)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  }

  BasicMultiDimensionalCartesianSpline::BasicMultiDimensionalCartesianSpline(std::string data_filename, bool CDFprovided):  _interpolator(data_filename)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  }

  BasicMultiDimensionalCartesianSpline::BasicMultiDimensionalCartesianSpline(const char * data_filename, bool CDFprovided):  _interpolator(data_filename)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  }

  BasicMultiDimensionalCartesianSpline::BasicMultiDimensionalCartesianSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta, bool CDFprovided):  _interpolator(discretizations, values, alpha, beta)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  }

  BasicMultiDimensionalCartesianSpline::BasicMultiDimensionalCartesianSpline(std::string data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided): _interpolator(data_filename, alpha, beta)
  {
    _CDFprovided = CDFprovided;
    BasicMultiDimensionalCartesianSpline_init();
  }

  BasicMultiDimensionalCartesianSpline::BasicMultiDimensionalCartesianSpline():_interpolator()
    {
    }


  void BasicMultiDimensionalCartesianSpline::BasicMultiDimensionalCartesianSpline_init(){
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
      int number_of_dimensions = discretizations.size();
      std::vector<int> discretizationSizes(number_of_dimensions);

      for (int i=0; i<number_of_dimensions; i++){
        numberofValues *= discretizations.at(i).size();
        discretizationSizes.at(i) = discretizations.at(i).size();
      }

      std::vector<double> CDFvalues(numberofValues);

      for (int i=0; i<numberofValues; i++){
        std::vector<int> nd_coordinateIndex = oneDtoNDconverter(i, discretizationSizes);
        std::vector<double> nd_coordinate(number_of_dimensions);
        for (int j=0; j<number_of_dimensions; j++)
          nd_coordinate.at(j) = discretizations.at(j)[nd_coordinateIndex.at(j)];
        CDFvalues.at(i) = _interpolator.integralSpline(nd_coordinate);
        //std::cout<< nd_coordinate.at(0) << " " << nd_coordinate.at(1) << " : " << CDFvalues.at(i) << std::endl;
      }
      _CDFinterpolator = NDSpline(discretizations,CDFvalues,alpha,beta);
    }
  }


  double
  BasicMultiDimensionalCartesianSpline::Pdf(std::vector<double> x)
  {
    if (_CDFprovided)
      return _interpolator.ndDerivative(x);
    else
      return _interpolator.interpolateAt(x);
  }

  double
  BasicMultiDimensionalCartesianSpline::Cdf(std::vector<double> x)
  {
    double value;

    if (_CDFprovided)
      value = _interpolator.interpolateAt(x);
    else
      value = _CDFinterpolator.interpolateAt(x);

    if (value > 1.0){
      std::cout<<x.at(0)<<" , "<<x.at(1) << " , " << value << std::endl;
      throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF value calculated is above 1.0");
    }

     return value;
  }


  std::vector<double>
  BasicMultiDimensionalCartesianSpline::InverseCdf(double F, double g)
  {
    if (_CDFprovided == true)
      return _interpolator.ndInverseFunctionGrid(F,g);
    else{
      return _CDFinterpolator.ndInverseFunctionGrid(F,g);
    }
  }

  double BasicMultiDimensionalCartesianSpline::inverseMarginal(double F, int dimension){
    double value=0.0;

    if ((F<1.0) and (F>0.0)){
      if (_CDFprovided){
        throwError("BasicMultiDimensionalCartesianSpline Distribution error: inverseMarginal calculation not available if CDF provided");
      }else{
        value = _interpolator.splineCartesianInverseMarginal(F, dimension, 0.01);
      }
    }else
      throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF value for inverse marginal distribution is above 1.0");

    return value;
  }

  int
  BasicMultiDimensionalCartesianSpline::returnDimensionality()
  {
    return _interpolator.returnDimensionality();
  }

//  double cellIntegral(std::vector<double> center, std::vector<double> dx){
//    if (_CDFprovided){
//      return _interpolator.averageCellValue(center,dx);
//    }else{
//      return _CDFinterpolator.averageCellValue(center,dx);
//    }
//  }

  void BasicMultiDimensionalCartesianSpline::updateRNGparameter(double tolerance, double initial_divisions){
    _tolerance = tolerance;
    _initial_divisions = (int)initial_divisions;

    if (_CDFprovided)
      _interpolator.updateRNGParameters(_tolerance,_initial_divisions);
    else{
      _interpolator.updateRNGParameters(_tolerance,_initial_divisions);
      _CDFinterpolator.updateRNGParameters(_tolerance,_initial_divisions);
    }
  }

  double BasicMultiDimensionalCartesianSpline::Marginal(double x, int dimension){
    double value=0.0;
    if (_CDFprovided){
      throwError("BasicMultiDimensionalCartesianSpline Distribution error: Marginal calculation not available if CDF provided");
    }else{
      value = _interpolator.splineCartesianMarginalIntegration(x, dimension);
    }
    return value;
  }

  double BasicMultiDimensionalCartesianSpline::returnUpperBound(int dimension){
    /**
     * this function returns the upper bound of the distribution for a particular dimension
     */
    return _interpolator.returnUpperBound(dimension);
  }

  double BasicMultiDimensionalCartesianSpline::returnLowerBound(int dimension){
   /**
    * this function returns the lower bound of the distribution for a particular dimension
    */
    return _interpolator.returnLowerBound(dimension);
  }

