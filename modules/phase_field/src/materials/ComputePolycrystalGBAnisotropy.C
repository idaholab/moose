//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePolycrystalGBAnisotropy.h"


registerMooseObject("PhaseFieldApp", ComputePolycrystalGBAnisotropy);

InputParameters
ComputePolycrystalGBAnisotropy::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Compute of the grain misorientation based on grian euler phi_1");
  params.addRequiredParam<UserObjectName>(
      "grain_tracker", "Name of GrainTracker user object that provides Grain ID according to element ID");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");  
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  return params;   
}

ComputePolycrystalGBAnisotropy::ComputePolycrystalGBAnisotropy(const InputParameters & parameters)
  : Material(parameters),
    _grain_tracker(getUserObject<GrainTracker>("grain_tracker")),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v")),
    _delta_theta(declareProperty<Real>("delta_theta"))
{
}

void
ComputePolycrystalGBAnisotropy::computeQpProperties()
{
  // Get list of active order parameters from grain tracker, std::vector<unsigned int>
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  unsigned int num_grain_id = 0; // the number of vaild graid IDs
  // std::vector<std::vector<unsigned int>> grainID_varibaleIndex; // Create a vector of grain IDs to order parameter indices
  std::vector<unsigned int> grainID; // Create a vector of grain IDs to 
  std::vector<unsigned int> varibaleIndex; // Create a vector of order parameter indices
  grainID.clear();
  varibaleIndex.clear();
  
  for (MooseIndex(op_to_grains) op_index = 0; op_index < op_to_grains.size(); ++op_index)
  {
    auto grain_id = op_to_grains[op_index];

    if (grain_id == FeatureFloodCount::invalid_id)
      continue;    
    
    // grainID_varibaleIndex[num_grain_id][0] = grain_id;
    // grainID_varibaleIndex[num_grain_id][1] = op_index; 
    grainID.push_back(grain_id);
    varibaleIndex.push_back(op_index);
    num_grain_id++;
  }
  
  if (grainID.size() == 1 || grainID.size() == 0) // inside the grain 
    _delta_theta[_qp] = 0; // the misorientation 
  else // on the grain boudary
  {
    Real sum_val = 0;
    Real sum_delta_theta = 0;
    Real Val = 0.0;
    for (unsigned int i = 0; i < grainID.size(); ++i)
    {
      for (unsigned int j = i+1; j < grainID.size(); ++j)
      {
        const RealVectorValue angles_i = _euler.getEulerAngles(grainID[i]); // Get the sequence of Euler angles for grain i
        const RealVectorValue angles_j = _euler.getEulerAngles(grainID[j]);

        unsigned int m = varibaleIndex[i]; // Get the order parameter index of grain i
        unsigned int n = varibaleIndex[j];

        Val = (100000.0 * ((*_vals[m])[_qp]) * ((*_vals[m])[_qp]) + 0.01) *
              (100000.0 * ((*_vals[n])[_qp]) * ((*_vals[n])[_qp]) + 0.01); // eta_i^2 * eta_j^2

        sum_val += Val;
        sum_delta_theta += std::abs(angles_i(0)-angles_j(0))*Val; // misorientation for only considering phi_1
      }
    }
    _delta_theta[_qp] = sum_delta_theta / sum_val;
  } 
}