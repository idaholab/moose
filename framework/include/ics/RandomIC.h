//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANDOMIC_H
#define RANDOMIC_H

#include "InitialCondition.h"
#include "RandomData.h"
#include "MooseRandom.h"

// Forward Declarations
class InputParameters;
class RandomIC;
namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<RandomIC>();

/**
 * RandomIC just returns a Random value.
 */
class RandomIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  RandomIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;
  void initialSetup() override;

protected:
  inline Real
  value_helper(dof_id_type id, MooseRandom & generator, std::map<dof_id_type, Real> & map)
  {
    auto it_pair = map.lower_bound(id);

    // Do we need to generate a new number?
    if (it_pair == map.end() || it_pair->first != id)
      it_pair = map.emplace_hint(it_pair, id, generator.rand(id));

    return it_pair->second;
  }

  const Real _min;
  const Real _max;
  const Real _range;

  const bool _is_nodal;
  const bool _use_legacy;

private:
  std::unique_ptr<RandomData> _elem_random_data;
  std::unique_ptr<RandomData> _node_random_data;

  MooseRandom * _elem_random_generator;
  MooseRandom * _node_random_generator;

  std::map<dof_id_type, Real> _elem_numbers;
  std::map<dof_id_type, Real> _node_numbers;
};

#endif // RANDOMIC_H
