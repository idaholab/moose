//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BndElement.h"
#include "ElemExtrema.h"

/**
 * A specialized ConstBndElement to be used in ray tracing that
 * also holds the element extrema intersection information
 */
struct TraceRayBndElement : ConstBndElement
{
  TraceRayBndElement(const Elem * elem,
                     const unsigned short side,
                     const BoundaryID bnd_id,
                     const ElemExtrema & extrema)
    : ConstBndElement(elem, side, bnd_id), extrema(extrema)
  {
  }

  ElemExtrema extrema;
};
