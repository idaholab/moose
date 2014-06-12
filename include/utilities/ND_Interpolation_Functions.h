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

bool checkUpperBound(double upper_bound, std::vector<double> values);
bool checkLowerBound(double lower_bound, std::vector<double> values);

class NDInterpolation
{
public:
  virtual double interpolateAt(std::vector<double> point_coordinate);
  virtual double getGradientAt(std::vector<double> point_coordinate);
  virtual void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
  std::vector<double> NDinverseFunction(double F_min, double F_max);
  double NDderivative(std::vector<double> x);
  NDInterpolation();
  ~NDInterpolation();

protected:
  std::string _data_filename;
  bool        _completed_init;
  double minkowskiDistance(std::vector<double> point1, std::vector<double> point2, double p);
  double vectorNorm(std::vector<double> point, double p);
};

class NDSpline: public NDInterpolation
{
public:
  double interpolateAt(std::vector<double> point_coordinate);
  double getGradientAt(std::vector<double> point_coordinate);
  void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
  NDSpline(std::string filename, std::vector<double> alpha, std::vector<double> beta);
  NDSpline();
  ~NDSpline();
  bool checkUB(double upper_bound);
  bool checkLB(double lower_bound);

  bool checkBoundaries(std::vector<double> point);

private:
  std::vector< std::vector<double> > _discretizations;
  std::vector<double> _values;
  int _dimensions;
  std::vector<double> _spline_coefficients;
  std::vector<double> _hj;
  std::vector<double> _alpha;
  std::vector<double> _beta;

  std::vector<double> _min_disc;
  std::vector<double> _max_disc;

  //void initializeCoefficientsVector();
  void saveCoefficient(double value, std::vector<int> coefficient_coordinate);
  double retrieveCoefficient(std::vector<int> coefficient_coordinate);

  double spline_cartesian_interpolation(std::vector<double> point_coordinate);
  double getPointAtCoordinate(std::vector<int> coordinates);

  int fromNDto1Dconverter(std::vector<int> coordinate);
  std::vector<int> from1DtoNDconverter(int oneDcoordinate, std::vector<int> indexes);

  void calculateCoefficients();
  std::vector<double> fillArrayCoefficient(int n_dimensions, std::vector<double> & data, std::vector<int> & loop_locator);

  void from2Dto1Drestructuring(std::vector<std::vector<double> > & twoDdata, std::vector<double> & oneDdata);
  void from1Dto2Drestructuring(std::vector<std::vector<double> > & twoDdata, std::vector<double> & oneDdata, int spacing);

  double phi(double t);
  double u_k(double x, double a, double h, double i);
  void tridag(std::vector<double> & a, std::vector<double> & b, std::vector<double> & c, std::vector<double> & r, std::vector<double> & u);
  std::vector<double> getCoefficients(std::vector<double> & y, double h, double alpha, double beta);
  //void iterationStep(int nDim, std::vector<double> & coefficients, std::vector<double> & data);

  std::vector<double> coefficientRestructuring(std::vector<std::vector<double> > matrix);
  std::vector<std::vector<double> > tensorProductInterpolation(std::vector<std::vector<double> > step1, double h, double alpha, double beta);
  std::vector<std::vector<double> > matrixRestructuring(std::vector<std::vector<double> > step1);
  std::vector<double> getValues(std::vector<int> & loop_locator);
};

class InverseDistanceWeighting: public NDInterpolation
{
public:
  double interpolateAt(std::vector<double> point_coordinate);
  double getGradientAt(std::vector<double> point_coordinate);
  void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
  std::vector<double> NDinverseFunction(double F_min, double F_max);
  InverseDistanceWeighting(std::string filename, double p);
  InverseDistanceWeighting(double p);
  bool checkUB(double upper_bound);
  bool checkLB(double lower_bound);

private:
  int _dimensions;
  int _number_of_points;
  double _p;
  std::vector<double> _values;
  std::vector< std::vector<double> > _point_coordinates;
};

class MicroSphere: public NDInterpolation
{
public:
  double interpolateAt(std::vector<double> point_coordinate);
  double getGradientAt(std::vector<double> point_coordinate);
  void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
  MicroSphere(std::string filename, double p, int precision);
  MicroSphere(double p, int precision);
private:
  int _dimensions;
  int _number_of_points;
  double _p;
  std::vector<double> _values;
  std::vector< std::vector<double> > _point_coordinates;
  int _precision;
  std::vector< std::vector<double> > _unit_vector;
  void MSinitialization();
  double cosValueBetweenVectors(std::vector<double> point1, std::vector<double> point2);
};


#endif
