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

#include "GrainDataTracker.h"
#include "DislocationDensityFileReader.h"

class DislocationDensityFileReader;

/**
 * Manage a list of dislocation densities for the grains
 **/
class GrainTrackerDislocations : public GrainDataTracker<Real>
{
public:
  GrainTrackerDislocations(const InputParameters & parameters);

  static InputParameters validParams();

  /// add a new grain and update the formation time
  virtual void newGrainCreated(unsigned int new_grain_id);

  /// report the time a grain was created
  Real getFormationTime(unsigned int grain_id) const;

protected:
  /// add mew grain information, called within newGrainCreated
  Real newGrain(unsigned int new_grain_id);

  /// If grains are added with a default supplied density or not
  const bool _default_density_grains;

  /// dislocation density default
  const Real _default_density;

  /// File Reader to grab dislocation densities
  const DislocationDensityFileReader & _density;

  /// additional per-grain formation time data
  std::vector<Real> _formation_data;
};
