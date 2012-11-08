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

#ifndef PENETRATIONAUX_H
#define PENETRATIONAUX_H

#include "AuxKernel.h"
#include "PenetrationLocator.h"


//Forward Declarations
class PenetrationAux;

template<>
InputParameters validParams<PenetrationAux>();

/**
 * Constant auxiliary value
 */
class PenetrationAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PenetrationAux(const std::string & name, InputParameters parameters);

  virtual ~PenetrationAux();

protected:
  enum PA_ENUM
  {
    PA_DISTANCE,
    PA_TANG_DISTANCE,
    PA_NORMAL_X,
    PA_NORMAL_Y,
    PA_NORMAL_Z,
    PA_CLOSEST_POINT_X,
    PA_CLOSEST_POINT_Y,
    PA_CLOSEST_POINT_Z,
    PA_ELEM_ID,
    PA_SIDE,
    PA_INCREMENTAL_SLIP_X,
    PA_INCREMENTAL_SLIP_Y,
    PA_INCREMENTAL_SLIP_Z
  };

  std::string _quantity_string;
  PA_ENUM _quantity;

  virtual Real computeValue();

  PenetrationLocator & _penetration_locator;
};

#endif //PENETRATIONAUX_H
