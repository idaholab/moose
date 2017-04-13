/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FAUXGRAINTRACKER_H
#define FAUXGRAINTRACKER_H

#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"

class FauxGrainTracker;

template <>
InputParameters validParams<FauxGrainTracker>();

/**
 * This class is a fake grain tracker object, it will not actually track grains nor remap them
 * but will provide the same interface as the grain tracker and can be used as a lightweight
 * replacement when neither of those methods are needed. You may safely use this object anytime
 * you have at least as many order parameters as you do grains.
 */
class FauxGrainTracker : public FeatureFloodCount, public GrainTrackerInterface
{
public:
  FauxGrainTracker(const InputParameters & parameters);
  virtual ~FauxGrainTracker();

  virtual void initialize() override;
  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void execute() override;

  // GrainTrackerInterface methods
  virtual Real getEntityValue(dof_id_type entity_id,
                              FeatureFloodCount::FieldType field_type,
                              std::size_t var_idx) const override;
  virtual const std::vector<unsigned int> &
  getVarToFeatureVector(dof_id_type elem_id) const override;
  virtual unsigned int getFeatureVar(unsigned int feature_id) const override;
  virtual std::size_t getNumberActiveGrains() const override;
  virtual std::size_t getTotalFeatureCount() const override;
  virtual Point getGrainCentroid(unsigned int grain_id) const override;
  virtual bool doesFeatureIntersectBoundary(unsigned int feature_id) const override;

private:
  /// The mapping of entities to grains, in this case always the order parameter
  std::map<dof_id_type, unsigned int> _entity_id_to_var_num;

  std::map<dof_id_type, std::vector<unsigned int>> _entity_var_to_features;
  std::vector<unsigned int> _empty_var_to_features;

  /// Used as the lightweight grain counter
  std::set<unsigned int> _variables_used;

  /// Total Grain Count
  std::size_t _grain_count;

  // Convenience variable holding the number of variables coupled into this object
  const std::size_t _n_vars;

  /// Used to emulate the tracking step of the real grain tracker object
  const int _tracking_step;

  /// Order parameter to grain indices (just a reflexive vector)
  std::vector<unsigned int> _op_to_grains;

  /// The volume of the feature
  std::map<unsigned int, Real> _volume;

  /// The count of entities contributing to the volume calculation
  std::map<unsigned int, unsigned int> _vol_count;

  /// The centroid of the feature (average of coordinates from entities participating in the volume calculation)
  std::map<unsigned int, Point> _centroid;
};

#endif
