//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "MooseTypes.h"

class FEProblemBase;
class MooseObject;
class Function;
class SinglePhaseFluidProperties;

/**
 * Interface class for objects that need the wall heat transfer coefficient of a lumped,
 * path-integrated flow segment from the Dittus-Boelter correlation.
 *
 * The segment is zero-dimensional, so the coefficient has no spatial dependence. The
 * consuming object supplies the flow state through the pure virtual methods below, which
 * it already holds for its own purposes; this interface adds only the fluid properties and
 * the flow channel geometry that the correlation needs on top of that state.
 */
class DittusBoelterHTCInterface
{
public:
  static InputParameters validParams();

  DittusBoelterHTCInterface(const MooseObject * moose_object);

protected:
  /**
   * Computes the wall heat transfer coefficient of the segment [W/(m^2-K)]
   *
   * @param t Time at which to evaluate the geometry of the flow channel
   */
  ADReal wallHeatTransferCoefficient(Real t) const;

  /// Mass flow rate through the segment [kg/s]
  virtual const ADReal & massFlowRate() const = 0;
  /// Fluid temperature of the segment [K]
  virtual const ADReal & fluidTemperature() const = 0;
  /// Fluid temperature at the nominal inlet neighbor [K]
  virtual const ADReal & inletFluidTemperature() const = 0;
  /// Fluid temperature at the nominal outlet neighbor [K]
  virtual const ADReal & outletFluidTemperature() const = 0;
  /**
   * Wetted perimeter of the flow channel [m]
   *
   * @param t Time at which to evaluate the wetted perimeter
   */
  virtual Real wettedPerimeter(Real t) const = 0;

private:
  /**
   * Gets the thread ID of the consuming object
   *
   * @param moose_object Consuming object
   */
  static THREAD_ID threadID(const MooseObject * moose_object);

  /**
   * Gets the function named by a parameter of the consuming object
   *
   * @param moose_object Consuming object
   * @param param_name Name of the parameter holding the function name
   */
  static const Function & getFunctionByParam(const MooseObject * moose_object,
                                             const std::string & param_name);

  /**
   * Gets the single-phase fluid properties object named by the 'fluid_properties' parameter
   * of the consuming object
   *
   * Reports the offending parameter if no such object exists, which looking the object up
   * through the problem directly would not.
   *
   * @param moose_object Consuming object
   */
  static const SinglePhaseFluidProperties &
  getFluidPropertiesOrError(const MooseObject * moose_object);

  /// Fluid properties of the flowing fluid
  const SinglePhaseFluidProperties & _fp_dbi;
  /// System reference pressure
  const Function & _P_ref_dbi;
  /// Cross-sectional area of the flow channel
  const Function & _flow_area_dbi;
};
