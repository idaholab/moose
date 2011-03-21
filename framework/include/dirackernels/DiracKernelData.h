#ifndef DIRACKERNELDATA_H
#define DIRACKERNELDATA_H

#include "Moose.h"
#include "Array.h"
// libMesh
#include "point.h"
#include "elem.h"
#include "numeric_vector.h"

namespace libMesh
{
  template <class T> class NumericVector;
}

namespace Moose
{
  
//Forward Declarations
class ArbitraryQuadrature;


class DiracKernelData //: public QuadraturePointData
{
public:
  DiracKernelData();
  virtual ~DiracKernelData();

  /**
   * Sets the points to evaluate at.
   */
  void setPoints(const std::vector<Point> & physical_points, const std::vector<Point> & mapped_points);

  void init();

  void reinit(const NumericVector<Number>& soln, const Elem * elem);

  /**
   * Interior test function.
   *
   * Note that there is a different test function for each variable... allowing for modified
   * basis for things like SUPG and GLS.
   */
  std::map<unsigned int, std::vector<std::vector<Real> > > _test;

  /**
   * The qrule that can hold arbitrary points in the reference element.
   */
  ArbitraryQuadrature * _arbitrary_qrule;

  /**
   * The points on the current element.
   */
  std::vector<Point> _current_points;
};

} // namespace

#endif //DIRACKERNELDATA_H
