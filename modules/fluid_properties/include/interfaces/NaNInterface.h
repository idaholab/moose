//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "MooseObject.h"

class NaNInterface;

template <>
InputParameters validParams<NaNInterface>();

/**
 * Interface class for producing errors, warnings, or just quiet NaNs
 *
 * For some objects it is desirable to continue running despite generation of
 * NaN(s). This class provides an interface for choosing whether to throw an
 * error, a warning, or nothing at all, just using a quiet NaN.
 */
class NaNInterface
{
public:
  NaNInterface(const MooseObject * moose_object);

protected:
  enum NaNMessage
  {
    NAN_MESSAGE_NONE = 0,
    NAN_MESSAGE_WARNING = 1,
    NAN_MESSAGE_ERROR = 2
  };

  const MooseObject * _moose_object;

  /// Raise mooseWarning or mooseError?
  const enum NaNMessage _emit_on_nan;

  /**
   * Produces errors, warnings, or just quiet NaNs
   */
  Real getNaN() const { return getNaN("A NaN was produced."); }

  template <typename... Args>
  Real getNaN(Args &&... args) const
  {
    switch (_emit_on_nan)
    {
      case (NAN_MESSAGE_WARNING):
        mooseWarning(_moose_object->name(), ": ", std::forward<Args>(args)...);
        break;
      case (NAN_MESSAGE_ERROR):
        mooseError(_moose_object->name(), ": ", std::forward<Args>(args)...);
        break;
      default:
        break;
    }
    // return a quiet NaN
    return std::nan("");
  }
};

