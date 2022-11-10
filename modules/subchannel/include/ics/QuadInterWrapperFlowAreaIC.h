/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "QuadSubChannelBaseIC.h"
#include "QuadSubChannelMesh.h"
#include "QuadInterWrapperBaseIC.h"

/**
 * This class calculates the cross-sectional flow area of the quadrilateral subchannel
 */
class QuadInterWrapperFlowAreaIC : public QuadInterWrapperBaseIC
{
public:
  QuadInterWrapperFlowAreaIC(const InputParameters & params);
  Real value(const Point & p);

public:
  static InputParameters validParams();
};
