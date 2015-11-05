/*
 * distributionNDBase.h
 * Created by @wangc on Oct. 23, 2015
 * Extracted from  distribution_base_ND.h
 *
 */
#ifndef DISTRIBUTION_ND_BASE_H
#define DISTRIBUTION_ND_BASE_H
#include <map>
#include <string>
#include <vector>
#include "ND_Interpolation_Functions.h"
#include "distributionFunctions.h"
#include <stdexcept>
#include <iostream>
#include <fstream>

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
   virtual std::vector<double> InverseCdf(double F, double g) = 0;

   virtual double inverseMarginal(double F, int dimension) = 0;
   virtual int returnDimensionality() = 0;
   double cellIntegral(std::vector<double> center, std::vector<double> dx);

   std::string & getType();

   std::vector<int> oneDtoNDconverter(int oneDcoordinate, std::vector<int> indexes){
     /**
      *  This function makes a conversion of a 1D array into an ND array.
      *  The objective it to determine the coordinates of an ND point from its coordinate in a 1D vector.
      *  The weights are needed since I do not know a priori the range of ND component.
      */
       int n_dimensions = indexes.size();
       std::vector<int> NDcoordinates (n_dimensions);
       std::vector<int> weights (n_dimensions);

       weights.at(0)=1;
       for (int nDim=1; nDim<n_dimensions; nDim++)
    weights.at(nDim)=weights.at(nDim-1)*indexes.at(nDim-1);

       for (int nDim=(n_dimensions-1); nDim>=0; nDim--){
    if (nDim>0){
      NDcoordinates.at(nDim) = oneDcoordinate/weights.at(nDim);
      oneDcoordinate -= NDcoordinates.at(nDim)*weights.at(nDim);
    }
    else
      NDcoordinates.at(0) = oneDcoordinate;
       }
       return NDcoordinates;
   };

protected:
   std::string _type; ///< Distribution type
   std::string _data_filename;
   EPbFunctionType _function_type;
   std::map <std::string,double> _dis_parameters;
   bool _checkStatus;

   double _tolerance;
   int _initial_divisions;

   //Marginal distribution functions
   //std::vector<NDInterpolation> marginalDistributions;
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


#endif /* DISTRIBUTION_ND_BASE_H */
