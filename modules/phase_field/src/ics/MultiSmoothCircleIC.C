//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiSmoothCircleIC.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", MultiSmoothCircleIC);

InputParameters
MultiSmoothCircleIC::validParams()
{
  InputParameters params = SmoothCircleBaseIC::validParams();
  params.addClassDescription("Random distribution of smooth circles with given minimum spacing");
  params.addRequiredParam<unsigned int>("numbub", "The number of bubbles to place");
  params.addRequiredParam<Real>("bubspac",
                                "minimum spacing of bubbles, measured from center to center");
  params.addParam<unsigned int>("numtries", 1000, "The number of tries");
  params.addRequiredParam<Real>("radius", "Mean radius value for the circles");
  params.addParam<Real>("radius_variation",
                        0.0,
                        "Plus or minus fraction of random variation in "
                        "the bubble radius for uniform, standard "
                        "deviation for normal");
  MooseEnum rand_options("uniform normal none", "none");
  params.addParam<MooseEnum>("radius_variation_type",
                             rand_options,
                             "Type of distribution that random circle radii will follow");
  return params;
}

MultiSmoothCircleIC::MultiSmoothCircleIC(const InputParameters & parameters)
  : SmoothCircleBaseIC(parameters),
    _numbub(getParam<unsigned int>("numbub")),
    _bubspac(getParam<Real>("bubspac")),
    _max_num_tries(getParam<unsigned int>("numtries")),
    _radius(getParam<Real>("radius")),
    _radius_variation(getParam<Real>("radius_variation")),
    _radius_variation_type(getParam<MooseEnum>("radius_variation_type"))
{
}

void
MultiSmoothCircleIC::initialSetup()
{
  // Set up domain bounds with mesh tools
  for (const auto i : make_range(Moose::dim))
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  // a variation is provided, but the type is set to 'none'
  if (_radius_variation > 0.0 && _radius_variation_type == 2)
    mooseError("If radius_variation > 0.0, you must pass in a radius_variation_type in "
               "MultiSmoothCircleIC");

  SmoothCircleBaseIC::initialSetup();
}

void
MultiSmoothCircleIC::computeCircleRadii()
{
  _radii.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    // Vary bubble radius
    switch (_radius_variation_type)
    {
      case 0: // Uniform distribution
        _radii[i] = _radius * (1.0 + (1.0 - 2.0 * _random.rand(_tid)) * _radius_variation);
        break;
      case 1: // Normal distribution
        _radii[i] = _random.randNormal(_tid, _radius, _radius_variation);
        break;
      case 2: // No variation
        _radii[i] = _radius;
    }

    _radii[i] = std::max(_radii[i], 0.0);
  }
}

void
MultiSmoothCircleIC::computeCircleCenters()
{
  _centers.resize(_numbub);
  for (unsigned int i = 0; i < _numbub; ++i)
  {
    // Vary circle center positions
    unsigned int num_tries = 0;
    while (num_tries < _max_num_tries)
    {
      num_tries++;

      RealTensorValue ran;
      ran(0, 0) = _random.rand(_tid);
      ran(1, 1) = _random.rand(_tid);
      ran(2, 2) = _random.rand(_tid);

      _centers[i] = _bottom_left + ran * _range;

      for (unsigned int j = 0; j < i; ++j)
        if (_mesh.minPeriodicDistance(_var.number(), _centers[j], _centers[i]) < _bubspac)
          goto fail;

      // accept the position of the new center
      goto accept;

    // retry a new position until tries are exhausted
    fail:
      continue;
    }

    if (num_tries == _max_num_tries)
      mooseError("Too many tries in MultiSmoothCircleIC");

  accept:
    continue;
  }
}
