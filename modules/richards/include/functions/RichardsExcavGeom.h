/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
  RealVectorValue _retreat_vel;


};

#endif //RICHARDSEXCAVGEOM

