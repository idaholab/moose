/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSEXCAVGEOM
#define RICHARDSEXCAVGEOM

#include "Function.h"

// Forward Declarations
class RichardsExcavGeom;

template<>
InputParameters validParams<RichardsExcavGeom>();

class RichardsExcavGeom : public Function
{
public:

  RichardsExcavGeom(const std::string & name,
                        InputParameters parameters);

  virtual Real value(Real t, const Point & p);

protected:

  RealVectorValue _start_posn;
  Real _start_time;
  RealVectorValue _end_posn;
  Real _end_time;
  Real _active_length;
  Real _true_value;
  RealVectorValue _retreat_vel;
  Real _norm_retreat_vel;


};

#endif //RICHARDSEXCAVGEOM

