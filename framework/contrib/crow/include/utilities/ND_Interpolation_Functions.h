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
 *      This class contains a set of multi-dimensional interpolation functions for both scattered data and data lying on a cartesian grid
 *
 *      Sources:
 *      - General
 *        * Numerical Recipes in C++ 3rd edition
 *      - MD spline
 *        * Christian Habermann, Fabian Kindermann, "Multidimensional Spline Interpolation: Theory and Applications", Computational Economics, Vol.30-2, pp 153-169 (2007) [http://link.springer.com/article/10.1007%2Fs10614-007-9092-4]
 *      - Inverse distance weighting
 *        * http://en.wikipedia.org/wiki/Inverse_distance_weighting
 *
 */


#ifndef ND_INTERPOLATION_FUNCTIONS_H
#define ND_INTERPOLATION_FUNCTIONS_H

#include <vector>
#include <string>
#include <iostream>
#include "randomClass.h"

bool checkUpperBound(double upper_bound, std::vector<double> values);
bool checkLowerBound(double lower_bound, std::vector<double> values);
std::vector<double> intToBinary(int value, int size);

class NDInterpolation
{
public:
  virtual double interpolateAt(std::vector<double> point_coordinate);
  virtual double getGradientAt(std::vector<double> point_coordinate);
  virtual void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
  //std::vector<double> NDinverseFunction(double f_min, double f_max);
  std::vector<double> ndInverseFunctionGrid(double f, double g);

  double averageCellValue(std::vector<double> center, std::vector<double> dx);

  void updateRNGParameters(double tolerance, double initial_divisions);

  double ndDerivative(std::vector<double> coordinate);

  int returnDimensionality(){return _dimensions;};

  double returnUpperBound(int dimension){return _upper_bound.at(dimension);};
  double returnLowerBound(int dimension){return _lower_bound.at(dimension);};

  NDInterpolation();
  virtual ~NDInterpolation();

protected:
  RandomClass * _random_double;
  std::string _data_filename;
  bool _completed_init;
  int _dimensions;

  double _tolerance;
  int _initial_divisions;

  std::vector<double> _cell_point_0;
  std::vector<double> _cell_dxs;

  std::vector<double> _upper_bound;
  std::vector<double> _lower_bound;

  double minkowskiDistance(std::vector<double> point1, std::vector<double> point2, double p);
  double vectorNorm(std::vector<double> point, double p);

  bool pivotCellCheck(std::vector<std::vector<double> >& cell, double f);
  int vertexOutcome(std::vector<double>& vertex, double f);
  void cellsFilter(std::vector<std::vector<std::vector<double> > >& vertices, double f);
  void refinedCellDivision(std::vector<std::vector<std::vector<double> > >& refined_cell, std::vector<std::vector<double> > & cell, int divisions);
  std::vector<int> arrayConverter(int one_d_coordinate, int divisions, int n_dimensions);
  std::vector<std::vector<double> > generateNewCell(std::vector<int> nd_coordinate, std::vector<double> coordinate_of_point_zero, std::vector<double> dxs, int n_dimensions);
  std::vector<std::vector<double> > pickNewCell(std::vector<std::vector<std::vector<double> > > & cells_set, double g);
  std::vector<double> getCellCenter(std::vector<std::vector<double> > & cell);

  double oneDDerivative(double fxph, double fx, double fxmh);
  double derivativeStep(std::vector<double> coordinate, int loop);

  int cdfWeightedPicking(std::vector<std::vector<std::vector<double> > >& vertices, double g);
  double integralCellValue(std::vector<std::vector<double> > cell);
};

/*
class genericNDSpline: public NDInterpolation{
public:
  double interpolateAt(std::vector<double> point_coordinate);
  double integralSpline(std::vector<double> point_coordinate);
  double splineCartesianMarginalIntegration(double coordinate,int marginal_variable);
  double splineCartesianInverseMarginal(double CDF,int marginal_variable, double precision);

  genericNDSpline(std::string filename);
  genericNDSpline(std::string filename, std::vector<double> alpha, std::vector<double> beta);
  genericNDSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta);
  void genericNDSpline_init(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta);

  genericNDSpline();
  virtual ~genericNDSpline();

private:
  std::vector< std::vector<double> > _discretizations;
  std::vector<double> _values;
  std::vector<double> _alpha;
  std::vector<double> _beta;
};
*/

class NDSpline: public NDInterpolation
{
public:
  double interpolateAt(std::vector<double> point_coordinate);
  double getGradientAt(std::vector<double> point_coordinate);
  double integralSpline(std::vector<double> point_coordinate);
  double splineCartesianMarginalIntegration(double coordinate,int marginal_variable);
  double splineCartesianInverseMarginal(double cdf,int marginal_variable, double precision);

  // std::vector<double> ndInverseFunctionGrid(double f, double g);

  void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);

  NDSpline(std::string filename);
  NDSpline(std::string filename, std::vector<double> alpha, std::vector<double> beta);
  NDSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta);
  void ndSplineInit(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta);

  //std::vector< std::vector<double> > getDiscretizations(){
  //      std::cout<<"but why!"<< std::endl;
  //      return _discretizations;};

  void getDiscretizations(std::vector< std::vector<double> > & vector){
          for(unsigned int i=0; i<_discretizations.size();i++){
                  std::vector<double> temp;
                  for(unsigned int j=0; j<_discretizations.at(i).size(); j++)
                          temp.push_back(_discretizations.at(i).at(j));
                  vector.push_back(temp);
          };
  }

  void printFunction(){
          std::cout<<"data ND spline1 value " << _discretizations.at(0).at(0) << std::endl;
          std::cout<<"data ND spline2 dimensions " << _dimensions << std::endl;
  };

  NDSpline();
  virtual ~NDSpline();

  bool checkUB(double upper_bound);
  bool checkLB(double lower_bound);

  bool checkBoundaries(std::vector<double> point);

  double getH(int dim){return _hj.at(dim);};

private:
  std::vector< std::vector<double> > _discretizations;
  std::vector<double> _values;
  std::vector<double> _spline_coefficients;
  std::vector<double> _hj;
  std::vector<double> _alpha;
  std::vector<double> _beta;

  std::vector<double> _min_disc;
  std::vector<double> _max_disc;

  //void initializeCoefficientsVector();
  void saveCoefficient(double value, std::vector<int> coefficient_coordinate);
  double retrieveCoefficient(std::vector<int> coefficient_coordinate);

  double splineCartesianInterpolation(std::vector<double> point_coordinate);
  double splineCartesianIntegration(std::vector<double> point_coordinate);
  double getPointAtCoordinate(std::vector<int> coordinates);

  int fromNDTo1DConverter(std::vector<int> coordinate);
  std::vector<int> from1DToNDConverter(int one_d_coordinate, std::vector<int> indexes);

  void calculateCoefficients();
  std::vector<double> fillArrayCoefficient(int n_dimensions, std::vector<double> & data, std::vector<int> & loop_locator);

  void from2DTo1DRestructuring(std::vector<std::vector<double> > & two_d_data, std::vector<double> & one_d_data);
  void from1DTo2DRestructuring(std::vector<std::vector<double> > & two_d_data, std::vector<double> & one_d_data, int spacing);

  double phi(double t);
  double phiDeriv(double t);
  double uk(double x, std::vector<double> & discretizations, double k);
  double ukDeriv(double x, std::vector<double> & discretizations, double k);
  void tridag(std::vector<double> & a, std::vector<double> & b, std::vector<double> & c, std::vector<double> & r, std::vector<double> & u);
  std::vector<double> getCoefficients(std::vector<double> & y, double h, double alpha, double beta);
  //void iterationStep(int nDim, std::vector<double> & coefficients, std::vector<double> & data);

  std::vector<double> coefficientRestructuring(std::vector<std::vector<double> > matrix);
  std::vector<std::vector<double> > tensorProductInterpolation(std::vector<std::vector<double> > step1, double h, double alpha, double beta);
  std::vector<std::vector<double> > matrixRestructuring(std::vector<std::vector<double> > step1);
  std::vector<double> getValues(std::vector<int> & loop_locator);

  /**
   * These functions are implemented in NDspline.C.
   * They implement the integral of the kernel functions of the ND-spline
   * which are needed to calculate CDF and marginal distributions.
   * Six functions are needed since the kernel function is piecewise with modulus operator.
   */

  double val1(double t);
  double val2(double t);
  double val3(double t);
  double val4(double t);
  double val5(double t);
  double val6(double t);

};

class InverseDistanceWeighting: public NDInterpolation
{
public:
  double interpolateAt(std::vector<double> point_coordinate);
  double getGradientAt(std::vector<double> point_coordinate);
  void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
  //std::vector<double> NDinverseFunction(double f_min, double f_max);
  InverseDistanceWeighting(std::string filename, double p);
  InverseDistanceWeighting(double p);
  virtual ~InverseDistanceWeighting() {};
  bool checkUB(double upper_bound);
  bool checkLB(double lower_bound);

  std::vector<double> getCellPoint0(){return _cell_point_0;};
  std::vector<double> getCellDxs(){return _cell_dxs;};

private:
  int _number_of_points;
  double _p;
  std::vector<double> _values;
  //std::vector<double> _cell_point_0;
  //std::vector<double> _cell_dxs;
  std::vector< std::vector<double> > _point_coordinates;
};

//class NDlinear: public NDInterpolation
//{
//public:
//  double interpolateAt(std::vector<double> point_coordinate);
//  NDlinear(std::string filename);
//  NDlinear();
//  ~NDlinear();
//  double linear_interpolation(std::vector<double> point_coordinate);
//  std::vector<double> getValues(std::vector<int> & loopLocator);
//  int fromNDTo1DConverter(std::vector<int> coordinate);
//  bool checkBoundaries(std::vector<double> point);
//  std::vector<int> from1DToNDConverter(int one_d_coordinate, std::vector<int> indexes);
//
//  bool checkUB(double upper_bound);
//  bool checkLB(double lower_bound);
//
//private:
//  int _dimensions;
//  int _number_of_points;
//  bool _completedInit;
//  double _p;
//  std::vector<double> _values;
//  std::vector<double> _minDisc;
//  std::vector<double> _maxDisc;
//  std::vector< std::vector<double> > _discretizations;
//};

class MicroSphere: public NDInterpolation
{
public:
  double interpolateAt(std::vector<double> point_coordinate);
  double getGradientAt(std::vector<double> point_coordinate);
  void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
  MicroSphere(std::string filename, double p, int precision);
  MicroSphere(double p, int precision);
  virtual ~MicroSphere() {};
private:
  unsigned int _number_of_points;
  double _p;
  std::vector<double> _values;
  std::vector< std::vector<double> > _point_coordinates;
  int _precision;
  std::vector< std::vector<double> > _unit_vector;
  void msInitialization();
  double cosValueBetweenVectors(std::vector<double> point1, std::vector<double> point2);
};


#endif
