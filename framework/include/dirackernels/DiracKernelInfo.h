#ifndef DIRACKERNELINFO_H_
#define DIRACKERNELINFO_H_

#include <set>
#include <map>

#include "Moose.h"
#include "Array.h"
// libMesh
#include "elem.h"
#include "point.h"

namespace libMesh
{
  template <class T> class NumericVector;
}

namespace Moose {

class DiracKernelInfo
{
public:
  DiracKernelInfo();
  virtual ~DiracKernelInfo();

public:
  void addPoint(const Elem * elem, Point p);

  /**
   * The list of elements that need distributions.
   */
  std::set<const Elem *> _elements;

  /**
   * The list of physical xyz Points that need to be evaluated in each element.
   */
  std::map<const Elem *, std::set<Point> > _points;

  /**
   * Remove all of the current points and elements.
   */
  void clearPoints();
};

} // namespace

#endif //DIRACKERNELINFO_H_
