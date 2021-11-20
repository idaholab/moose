/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#pragma once

#include "ACGrGrBase.h"
#include "GrainTrackerDislocations.h"


/**
 * This kernel calculates the residual for grain growth driven by dislocation density.
 * */
class ACPolycrystalDislocationEnergy : public ACGrGrBase
{
public:
  ACPolycrystalDislocationEnergy(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeDFDOP(PFFunctionType type);

private:
  /// shear modulus
  const MaterialProperty<Real> & _G;

  /// burgers vector
  const MaterialProperty<Real> & _b;

  /// dislocation density
  const MaterialProperty<Real> & _dislocation_density;

  /// sum of h(eta_i)
  const MaterialProperty<Real> & _sum_h_OP;

  /// Grain Tracker object for dislocation information
  const GrainDataTracker<Real> & _grain_tracker;

  /// which OP is being operated on
  const unsigned int _op_index;

  /// dislocation data
  const MaterialProperty<std::vector<Real>> & _grain_disloc_data;
};
