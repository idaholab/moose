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
  static InputParameters validParams();

  NaNInterface(const MooseObject * moose_object);

protected:
  enum NaNMessage
  {
    NAN_MESSAGE_NONE = 0,
    NAN_MESSAGE_WARNING = 1,
    NAN_MESSAGE_ERROR = 2
  };

  const MooseObject * const _moose_object;

  /// Raise mooseWarning or mooseError?
  const enum NaNMessage _emit_on_nan;

  /**
   * Throws an error or returns a NaN with or without a warning, with a default message
   */
  Real getNaN() const { return getNaN("A NaN was produced."); }

  /**
   * Throws an error or returns NaNs with or without a warning, with a default message
   *
   * @param[in] n   Vector size
   */
  std::vector<Real> getNaNVector(const unsigned int & n) const
  {
    return getNaNVector(n, "A NaN was produced.");
  }

  /**
   * Throws an error or returns a NaN with or without a warning
   */
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

  /**
   * Throws an error or returns NaNs with or without a warning
   *
   * @param[in] n   Vector size
   */
  template <typename... Args>
  std::vector<Real> getNaNVector(const unsigned int & n, Args &&... args) const
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
    // return quiet NaNs
    return std::vector<Real>(n, std::nan(""));
  }
};
