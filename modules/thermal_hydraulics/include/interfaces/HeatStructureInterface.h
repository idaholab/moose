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

class HeatConductionModel;
class GeometricalComponent;

/**
 * Interface class for heat structure components
 */
class HeatStructureInterface
{
public:
  static InputParameters validParams();

  HeatStructureInterface(GeometricalComponent * geometrical_component);

  /**
   * Gets the initial temperature function name
   */
  FunctionName getInitialT() const;

  /**
   * Gets the geometrical component inheriting from this interface
   */
  const GeometricalComponent & getGeometricalComponent() const
  {
    return _geometrical_component_hsi;
  }

protected:
  /**
   * Use cylindrical transformation?
   */
  virtual bool useCylindricalTransformation() const = 0;

  /**
   * Builds the heat conduction model
   */
  virtual std::shared_ptr<HeatConductionModel> buildModel();

  /**
   * Method to be called in the component's init() method
   */
  void init();

  /**
   * Method to be called in the component's check() method
   */
  void check() const;

  /**
   * Method to be called in the component's addVariables() method
   */
  void addVariables();

  /**
   * Method to be called in the component's addMooseObjects() method
   */
  void addMooseObjects();

  /// The heat conduction model used by this heat structure
  std::shared_ptr<HeatConductionModel> _hc_model;

private:
  /// The geometrical component inheriting from this interface
  GeometricalComponent & _geometrical_component_hsi;
};
