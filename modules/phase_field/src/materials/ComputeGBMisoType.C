//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeGBMisoType.h"
#include "SolutionUserObject.h"
#include <fstream>

registerMooseObject("PhaseFieldApp", ComputeGBMisoType);

InputParameters
ComputeGBMisoType::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculate types of grain boundaries in a polycrystalline sample");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addRequiredParam<FileName>("file_name", "Misorientation angle data file name");
  params.addParam<Real>("angle_threshold", 15, "Max LAGB Misorientation angle");
  return params;
}

ComputeGBMisoType::ComputeGBMisoType(const InputParameters & parameters)
  : Material(parameters),
    _grain_tracker(getUserObject<GrainTracker>("grain_tracker")),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _file_name(getParam<FileName>("file_name")),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v")),
    _angle_threshold(getParam<Real>("angle_threshold")),
    _gb_type(declareADProperty<Real>("gb_type"))
{
  readFile();
}

void
ComputeGBMisoType::computeQpProperties()
{

  // Find out the number of boundary unique_id and save them
  std::vector<unsigned int> gb_pairs;
  std::vector<Real> gb_op_pairs;
  MooseIndex(_vals) val_index = -1;

  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());
  for (auto & unique_id : op_to_grains)
  {
    val_index+=1;
    if (unique_id == FeatureFloodCount::invalid_id)
      continue;

    auto grain_id = _ebsd_reader.getFeatureID(unique_id);

    gb_pairs.push_back(grain_id);
    gb_op_pairs.push_back((*_vals[val_index])[_qp]);
  }

  // Compute GB type by the number of id
  switch (gb_pairs.size())
  {
    case 0:
    {
      break;
    }
    case 1:
    {
      _gb_type[_qp] = 0;
      break;
    }
    case 2:
    {
      // get type by miso angle
      _gb_type[_qp] = ((_miso_angles[getLineNum(gb_pairs[0], gb_pairs[1])] < _angle_threshold) ? 1 : 2);
      break;
    }
    default:
    // get continuous type at triple junction
      _gb_type[_qp] = getTripleJunctionType(gb_pairs, gb_op_pairs);
  }

}

// Read the miso angle file
void
ComputeGBMisoType::readFile()
{
  // Read in Euler angles from _file_name
  std::ifstream inFile(_file_name.c_str());
  if (!inFile)
    mooseError("Can't open ", _file_name);

  // Loop over misorientation angles
  Real a;
  while (inFile >> a)
    _miso_angles.push_back(a);
}

// Function to output total line number of miso angle file
unsigned int
ComputeGBMisoType::getTotalLineNum() const
{
  return _miso_angles.size();
}

// Function to output specific line number in miso angle file
unsigned int
ComputeGBMisoType::getLineNum(unsigned int grain_i, unsigned int grain_j)
{
  if (grain_i > grain_j) return grain_j + (grain_i - 1) * grain_i / 2;
  else return grain_i + (grain_j - 1) * grain_j / 2;
}

// Function to calculate the GB type in Triple junction
Real
ComputeGBMisoType::getTripleJunctionType(std::vector<unsigned int> gb_pairs,
                                         std::vector<Real> gb_op_pairs)
{
  unsigned int lagb_num = 0;
  unsigned int hagb_num = 0;
  Real ratio_base = 0.0;
  Real ratio_lagb = 0.0;
  for (unsigned int i=1; i<gb_pairs.size(); ++i){
    for (unsigned int j=0; j<i; ++j){
      ratio_base += (gb_op_pairs[i]*gb_op_pairs[i]
                    *gb_op_pairs[j]*gb_op_pairs[j]);
      if (_miso_angles[getLineNum(gb_pairs[j], gb_pairs[i])] < _angle_threshold) {
        lagb_num += 1;
        ratio_lagb += (gb_op_pairs[i]*gb_op_pairs[i]
                      *gb_op_pairs[j]*gb_op_pairs[j]);
      }
      else
        hagb_num += 1;
    }
  }
  if (lagb_num == 0)
    return 2;
  else if (hagb_num == 0)
    return 1;
  else
    return 2 - ratio_lagb/ratio_base;
}
