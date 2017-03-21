/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRAINDATATRACKER_H
#define GRAINDATATRACKER_H

#include "GrainTracker.h"

/**
 * GrainTracker derived class template to base objects on which maintain physical
 * parameters for individual grains.
 */
template <typename T>
class GrainDataTracker : public GrainTracker
{
public:
  GrainDataTracker(const InputParameters & parameters);

  /// return data for selected grain
  const T & getData(unsigned int grain_id) const;

protected:
  /// implement this method to initialize the data for the new grain
  virtual T newGrain(unsigned int new_grain_id) = 0;

  virtual void newGrainCreated(unsigned int new_grain_id);

  /// per grain data
  std::vector<T> _grain_data;
};

template <typename T>
GrainDataTracker<T>::GrainDataTracker(const InputParameters & parameters) : GrainTracker(parameters)
{
}

template <typename T>
const T &
GrainDataTracker<T>::getData(unsigned int grain_id) const
{
  mooseAssert(grain_id < _grain_data.size(), "Requested data for invalid grain index.");
  return _grain_data[grain_id];
}

template <typename T>
void
GrainDataTracker<T>::newGrainCreated(unsigned int new_grain_id)
{
  if (_grain_data.size() <= new_grain_id)
    _grain_data.resize(new_grain_id + 1);

  _grain_data[new_grain_id] = newGrain(new_grain_id);
}

#endif // GRAINDATATRACKER_H
