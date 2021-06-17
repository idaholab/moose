//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LotsOfRaysRayStudy.h"

/**
 * Test Ray study that generates a lot of Rays and sets data/aux data
 * with a predictable pattern in conjunction with TestRayDataRayKernel
 * that allows for verification of the data at the end of the trace
 */
class TestRayDataStudy : public LotsOfRaysRayStudy
{
public:
  TestRayDataStudy(const InputParameters & parameters);

  static InputParameters validParams();

  RayData dataValue(const unsigned int i, const Ray & ray) const;
  RayData dataValueChange(const unsigned int i, const Real distance) const;
  RayData auxDataValue(const unsigned int i, const Ray & ray) const;

  virtual void onCompleteRay(const std::shared_ptr<Ray> & ray) override;

  const std::vector<RayDataIndex> & dataIndices() const { return _data_indices; }

protected:
  virtual void modifyRays() override;

  const std::size_t _data_size;
  const std::size_t _aux_data_size;

private:
  std::vector<RayDataIndex> _data_indices;
  std::vector<RayDataIndex> _aux_data_indices;
  std::vector<RayDataIndex> _actual_start_indices;
};
