/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSPOLYLINESINK_H
#define RICHARDSPOLYLINESINK_H

#include "DiracKernel.h"
#include "LinearInterpolation.h"
#include "RichardsSumQuantity.h"

//Forward Declarations
class RichardsPolyLineSink;

template<>
InputParameters validParams<RichardsPolyLineSink>();

/**
 * Approximates a polyline by a sequence of Dirac Points
 * the mass flux from each Dirac Point is _sink_func as a
 * function of porepressure at the Dirac Point.
 */
class RichardsPolyLineSink : public DiracKernel
{
public:
  RichardsPolyLineSink(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();


protected:

  /**
   * This is used to hold the total fluid flowing into the sink
   * Hence, it is positive for sinks where fluid is flowing
   * from porespace into the borehole and removed from the model
   */
  RichardsSumQuantity & _total_outflow_mass;

  /// mass flux = _sink_func as a function of porepressure
  LinearInterpolation _sink_func;

  /// contains rows of the form x y z (space separated)
  std::string _point_file;

  /**
   * if true then Dirac Points are added to the mesh at the start,
   * and then containing elements are cached.
   * if false then the containing elements are computed each time step
   * which can be quite expensive
   */
  bool _mesh_adaptivity;

  /// vector of Dirac Points' x positions
  std::vector<Real> _xs;

  /// vector of Dirac Points' y positions
  std::vector<Real> _ys;

  /// vector of Dirac Points' z positions
  std::vector<Real> _zs;

  /// the elements that contain the Dirac Points
  std::vector<const Elem *> _elemental_info;

  /// whether have constructed _elemental_info
  bool _have_constructed_elemental_info;

  /**
   * reads a space-separated line of floats from ifs and puts in myvec
   * @param ifs the file stream
   * @param myvec upon return will contain the space-separated flows encountered in ifs
   */
  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec);
};

#endif //RICHARDSPOLYLINESINK_H
