//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class InputParameters;
class ThermalHydraulicsApp;

/**
 * Interface class for accessing the ThermalHydraulicsApp
 */
class THMAppInterface
{
public:
  static InputParameters validParams();

  THMAppInterface(const InputParameters & params);

protected:
  /**
   * Gets the ThermalHydraulicsApp
   */
  ThermalHydraulicsApp & getTHMApp() const { return _thm_app; }

private:
  /**
   * Initializes the ThermalHydraulicsApp
   *
   * @param[in] moose_app  The MooseApp
   */
  ThermalHydraulicsApp & initializeThermalHydraulicsAppReference(const InputParameters & params);

  /// The ThermalHydraulicsApp
  ThermalHydraulicsApp & _thm_app;
};
