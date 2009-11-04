#ifndef POLYNOMIALFIT_H
#define POLYNOMIALFIT_H

#include <vector>
#include <fstream>
#include <sstream>
#include <string>


/**
 * This class applies the Least Squares algorithm to a set of points to provide a smooth curve for
 * sampling values.
 *
 * Requires: LAPACK
 */
class PolynomialFit
{
public:

  /* Constructor, Takes two vectors of points for which to apply the fit.  One should be of the
   * independant variable while the other should be of the dependant varible.  These values should
   * correspond to one and other in the same position.  The third parameter is the requested polynomial
   * order and the forth parameter tells the class whether or not it should truncate the order if there
   * are not enough points for which to apply the polynomial fit.
   */
  PolynomialFit(std::vector<double> X, std::vector<double> Y, unsigned int order, bool truncate_order = false);

  virtual ~PolynomialFit()
    {}

  /**
   * This function generates the polynomial fit.  This function must be called prior to using
   * sample or dumpSample File. Note:  If you pass a vectors that
   * contain duplicate independant measures the call to LAPACK will fail
   */
  void generate();

  /**
   * This function will take an indenandant variable input and will return the dependant variable
   * based on the generated fit
   */
  double sample(double x);

  /**
   * This function will dump GNUPLOT input files that can be run to show the data points and
   * function fits
   */
  void dumpSampleFile(unsigned int proc_id, float xmin, float xmax, float ymin, float ymax);

  /**
   * This function returns the size of the array holding the points, i.e. the number of sample points
   */
  unsigned int getSampleSize();
  
private:

  /**
   * This is a helper function that creates the matrix necessary for the Least Squares algorithm.
   */
  void fillMatrix();

  /**
   * This function is the wrapper for the LAPACK dgels function and is called by generate.
   */
  void doLeastSquares();

  std::vector<double> _x;
  std::vector<double> _y;
  std::vector<double> _matrix;
  std::vector<double> _coeffs;
  unsigned int _order;
  bool _truncate_order;

  static int _file_number;
};

#endif //POLYNOMIALFIT_H
  
  
  
      
