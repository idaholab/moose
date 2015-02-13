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
   virtual std::vector<double> InverseCdf(double F) = 0;

   virtual double inverseMarginal(double F, int dimension) = 0;
   virtual int returnDimensionality() = 0;
   double cellIntegral(std::vector<double> center, std::vector<double> dx);

   std::string & getType();

   std::vector<int> oneDtoNDconverter(int oneDcoordinate, std::vector<int> indexes){
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



class BasicMultiDimensionalCartesianSpline: public  virtual BasicDistributionND
{
public:
  BasicMultiDimensionalCartesianSpline(const char * data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided):  _interpolator(data_filename,alpha, beta)
  {
	  _CDFprovided = CDFprovided;

	  BasicMultiDimensionalCartesianSpline(std::string(data_filename),alpha,beta,CDFprovided);
  };

  BasicMultiDimensionalCartesianSpline(std::string data_filename, bool CDFprovided):  _interpolator(data_filename)
  {
	  _CDFprovided = CDFprovided;

	  std::vector<double> alpha(_interpolator.returnDimensionality());
	  std::vector<double> beta(_interpolator.returnDimensionality());

	  for(int i=0; i<_interpolator.returnDimensionality(); i++){
		  alpha[i] = 0.0;
		  beta[i] = 0.0;
	  }

	  BasicMultiDimensionalCartesianSpline(data_filename,alpha,beta,CDFprovided);
  };

  BasicMultiDimensionalCartesianSpline(const char * data_filename, bool CDFprovided):  _interpolator(data_filename)
  {
	  _CDFprovided = CDFprovided;

	  std::vector<double> alpha(_interpolator.returnDimensionality());
	  std::vector<double> beta(_interpolator.returnDimensionality());

	  for(int i=0; i<_interpolator.returnDimensionality(); i++){
		  alpha[i] = 0.0;
		  beta[i] = 0.0;
	  }

	  BasicMultiDimensionalCartesianSpline(std::string(data_filename),alpha,beta,CDFprovided);
  };

  BasicMultiDimensionalCartesianSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta, bool CDFprovided):  _interpolator(discretizations, values, alpha, beta)
  {
	  _CDFprovided = CDFprovided;

	  if (_CDFprovided){
	   bool LBcheck = _interpolator.checkLB(0.0);
	   if (LBcheck == false)
		throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element below 0.0");

	   bool UBcheck = _interpolator.checkUB(1.0);
	   if (UBcheck == false)
		throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element above 1.0");
	  }

	  if (_CDFprovided == false){    // PDF provided ---> create grid for CDF

		  std::cerr<<"Creation of CDF interpolator for cartesian spline"<< std::endl;
		  std::vector< std::vector<double> > discretizations = _interpolator.getDiscretizations();
		  int numberofValues = 1;
		  int numberOfDimensions = discretizations.size();
		  std::vector<int> discretizationSizes(numberOfDimensions);
		  for (int i=0; i<numberOfDimensions; i++){
			  numberofValues *= discretizations[i].size();
			  discretizationSizes[i] = discretizations[i].size();
		  }

		  std::vector<double> CDFvalues(numberofValues);

		  for (int i=0; i<numberofValues; i++){
			  std::vector<int> NDcoordinateIndex = oneDtoNDconverter(i, discretizationSizes);
			  std::vector<double> NDcoordinate(numberOfDimensions);
			  for (int j=0; j<numberOfDimensions; j++)
				  NDcoordinate[j] = discretizations[j][NDcoordinateIndex[j]];
			  CDFvalues[i] = Cdf(NDcoordinate);
		  }

		  _CDFinterpolator = NDSpline(discretizations,CDFvalues,alpha,beta);
		  std::cerr<<"Creation of CDF interpolator for cartesian spline completed"<< std::endl;
	  }
  };

  BasicMultiDimensionalCartesianSpline(std::string data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided): _interpolator(data_filename, alpha, beta)
  {
	  _CDFprovided = CDFprovided;

	  if (_CDFprovided){
	   bool LBcheck = _interpolator.checkLB(0.0);
	   if (LBcheck == false)
		throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element below 0.0 in file: " << data_filename);

	   bool UBcheck = _interpolator.checkUB(1.0);
	   if (UBcheck == false)
		throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element above 1.0 in file: " << data_filename);
	  }

	  if (_CDFprovided == false){    // PDF provided ---> create grid for CDF

		  std::cerr<<"Creation of CDF interpolator for cartesian spline"<< std::endl;
		  std::vector< std::vector<double> > discretizations = _interpolator.getDiscretizations();
		  int numberofValues = 1;
		  int numberOfDimensions = discretizations.size();
		  std::vector<int> discretizationSizes(numberOfDimensions);
		  for (int i=0; i<numberOfDimensions; i++){
			  numberofValues *= discretizations[i].size();
			  discretizationSizes[i] = discretizations[i].size();
		  }

		  std::vector<double> CDFvalues(numberofValues);

		  for (int i=0; i<numberofValues; i++){
			  std::vector<int> NDcoordinateIndex = oneDtoNDconverter(i, discretizationSizes);
			  std::vector<double> NDcoordinate(numberOfDimensions);
			  for (int j=0; j<numberOfDimensions; j++)
				  NDcoordinate[j] = discretizations[j][NDcoordinateIndex[j]];
			  CDFvalues[i] = _interpolator.integralSpline(NDcoordinate);
		  }
		  _CDFinterpolator = NDSpline(discretizations,CDFvalues,alpha,beta);
		  std::cerr<<"Creation of CDF interpolator for cartesian spline completed"<< std::endl;
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
  InverseCdf(double F)
  {
	  if (_CDFprovided)
		  return _interpolator.NDinverseFunctionGrid(F);
	  else
		  return _CDFinterpolator.NDinverseFunctionGrid(F);
  };

  double inverseMarginal(double F, int dimension){
	  double value=0.0;

	  if ((F<1.0) and (F>0.0)){
		  if (_CDFprovided){
			  throwError("BasicMultiDimensionalCartesianSpline Distribution error: inverseMarginal calculation not available if CDF provided");
		  }else{
			  value = _interpolator.spline_cartesian_inverse_marginal(F, dimension, 0.00001);
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

  double cellIntegral(std::vector<double> center, std::vector<double> dx){
	  if (_CDFprovided){
		  return _interpolator.averageCellValue(center,dx);
	  }else{
		  return _CDFinterpolator.averageCellValue(center,dx);
	  }
  }


protected:
  bool _CDFprovided;
  NDSpline _interpolator;
  NDSpline _CDFinterpolator;
};



class BasicMultiDimensionalInverseWeight: public virtual BasicDistributionND
{
public:
  BasicMultiDimensionalInverseWeight(const char * data_filename,double p, bool CDFprovided):  _interpolator(data_filename,p)
  {
	  //_CDFprovided = CDFprovided;
	  BasicMultiDimensionalInverseWeight(std::string(data_filename),p,CDFprovided);
  };

  BasicMultiDimensionalInverseWeight(std::string data_filename,double p, bool CDFprovided):  _interpolator(data_filename,p)
  {
	  std::cerr<<"Initialize BasicMultiDimensionalInverseWeight"<< std::endl;

	  _CDFprovided = CDFprovided;

	  if (_CDFprovided) {
		  bool LBcheck = _interpolator.checkLB(0.0);
		  if (LBcheck == false)
			  throwError("BasicMultiDimensionalInverseWeight Distribution error: CDF values given as input contain element below 0.0 in file: " << data_filename);

		  bool UBcheck = _interpolator.checkUB(1.0);
		  if (UBcheck == false)
			  throwError("BasicMultiDimensionalInverseWeight Distribution error: CDF values given as input contain element above 1.0 in file: " << data_filename);
	  }
	  else{	// PDF is provided
		  // Create ND spline for the CDF
		  //BasicMultiDimensionalCartesianSpline(std::string data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided)
		  std::cerr<<"Creating ND spline for inverseWeight"<< std::endl;
		  int n_dimensions = _interpolator.returnDimensionality();
		  std::cerr<<"n_dimensions: "<<  n_dimensions << std::endl;
		  std::vector<double> alpha (n_dimensions);
		  std::vector<double> beta (n_dimensions);

		  for(int i=0; i<n_dimensions; i++){
			  alpha[i] = 0.0;
			  beta[i] = 0.0;
		  }

		  int numberDiscretization = 15;

		  //std::string ID = data_filename;
		  //std::string name="CDFdata_" + ID + ".txt";
		  //std::ofstream outputFile;
		  //outputFile.open(name.c_str());

		  //outputFile<<n_dimensions;

		  //for(int i=0; i<n_dimensions; i++)
		  //	  outputFile<<numberDiscretization;

		  std::vector<std::vector<double> > discretizations;
		  std::vector<double> cellPoint0 = _interpolator.get_cellPoint0();
		  std::vector<double> cellDxs = _interpolator.get_cellDxs();

		  for(int i=0; i<n_dimensions; i++){
			  std::vector<double> temp;
			  for(int j=0; j<numberDiscretization; j++){
				  double value = cellPoint0.at(i) + cellDxs.at(i)/numberDiscretization * j;
				  //outputFile<<value;
				  temp.push_back(value);
			  }
			  discretizations.push_back(temp);
		  }

		  int totalNumberOfValues=1;
		  std::vector<int> discretizationSizes(n_dimensions);
		  for(int i=0; i<n_dimensions; i++){
			  totalNumberOfValues *= numberDiscretization;
			  discretizationSizes[i] = numberDiscretization;
		  }

		  std::vector<double> PDFvalues (totalNumberOfValues);

		  for (int i=0; i<totalNumberOfValues; i++){
			  std::vector<int> NDcoordinateIndex = oneDtoNDconverter(i, discretizationSizes);
			  std::vector<double> NDcoordinate(n_dimensions);
			  for (int j=0; j<n_dimensions; j++)
				  NDcoordinate[j] = discretizations[j][NDcoordinateIndex[j]];
			  PDFvalues[i] = _interpolator.interpolateAt(NDcoordinate);
			  //outputFile<<PDFvalue;
		  }

		  _CDFspline = BasicMultiDimensionalCartesianSpline(discretizations, PDFvalues, alpha, beta, true);
		  //outputFile.close();
		  std::cerr<<"Creation of ND spline distribution for inverseWeight completed"<< std::endl;
	  }
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
	  if (_CDFprovided){
		  return _interpolator.NDderivative(x);
	  }
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
		  value = _interpolator.integral(x);

     if (value > 1.0)
    	 throwError("BasicMultiDimensionalInverseWeight Distribution error: CDF value calculated is above 1.0");

     return value;
  };

  void updateRNGparameter(double tolerance, double initial_divisions){
	  _tolerance = tolerance;
	  _initial_divisions = (int)initial_divisions;

	  _interpolator.updateRNGparameters(_tolerance,_initial_divisions);

	  std::cout<<"Distribution updateRNGparameter" <<std::endl;
  };

  std::vector<double>
  InverseCdf(double F)
  {
	  std::vector<double> value;

	  if (_CDFprovided)
		  value = _interpolator.NDinverseFunctionGrid(F);
	  else
		  value = _CDFspline.InverseCdf(F);

      return value;
  };

  double inverseMarginal(double F, int dimension){
	  double value=0.0;

	  if ((F<1.0) and (F>0.0)){
		  if (_CDFprovided){
			  throwError("BasicMultiDimensionalInverseWeight Distribution error: inverseMarginal calculation not available if CDF provided");
		  }else{
			  value = _CDFspline.inverseMarginal(F, dimension);
		  }
	  }else
		  throwError("BasicMultiDimensionalInverseWeight Distribution error: CDF value for inverse marginal distribution is above 1.0");

	  return value;
  }

  int
  returnDimensionality()
  {
	  return _interpolator.returnDimensionality();
  }


protected:
  InverseDistanceWeighting  _interpolator;
  BasicMultiDimensionalCartesianSpline  _CDFspline;
  bool _CDFprovided;
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

  int
  returnDimensionality()
  {
	  return _mu.size();
  }

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
  InverseCdf(double F)
  {
   return _interpolator.NDinverseFunctionGrid(F);
      //return _interpolator.NDinverseFunction(min, max);
  };

  int
  returnDimensionality()
  {
	  return _interpolator.returnDimensionality();
  }

protected:
  MicroSphere _interpolator;
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
