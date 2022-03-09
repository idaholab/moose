//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParisLaw.h"

// MOOSE includes
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "libmesh/system.h"

registerMooseObject("XFEMApp", ParisLaw);

InputParameters
ParisLaw::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Computes the crack extension size at all active crack front points.");
  params.addRequiredParam<Real>(
      "max_growth_size",
      "the max growth size at the crack front in each increment of a fatigue simulation");
  params.addRequiredParam<Real>("paris_law_c", "parameter C in the Paris law for fatigue");
  params.addRequiredParam<Real>("paris_law_m", "parameter m in the Paris law for fatigue");
  return params;
}

ParisLaw::ParisLaw(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _cutter(&_fe_problem.getUserObject<CrackMeshCut3DUserObject>("cut_mesh")),
    _max_growth_size(getParam<Real>("max_growth_size")),
    _paris_law_c(getParam<Real>("paris_law_c")),
    _paris_law_m(getParam<Real>("paris_law_m"))
{
}

void
ParisLaw::initialize()
{
}

void
ParisLaw::execute()
{
  // Generate _active_boundary and _inactive_boundary_pos;
  // This is a duplicated call before the one in CrackMeshCut3DUserObject;
  // That one cannot be deleted because this one is for subcritical cracking only.
  _cutter->findActiveBoundaryNodes();

  std::vector<int> index = _cutter->getFrontPointsIndex();

  const VectorPostprocessorValue & k1 = getVectorPostprocessorValueByName("II_KI_1", "II_KI_1");
  const VectorPostprocessorValue & k2 = getVectorPostprocessorValueByName("II_KII_1", "II_KII_1");
  mooseAssert(k1.size() == k2.size(), "KI and KII VPPs should have the same size");
  unsigned int size_this_segment = k1.size();

  _effective_k.clear();

  for (unsigned int i = 0; i < size_this_segment; ++i)
  {
    int ind = index[i];
    if (ind == -1)
      _effective_k.push_back(0.0);
    else if (ind >= 0)
    {
      Real effective_k = sqrt(pow(k1[ind], 2) + 2 * pow(k2[ind], 2));
      _effective_k.push_back(effective_k);
    }
    else
      mooseError("index must be either -1 (inactive) or >= 0 (active)");
  }

  Real _max_k = *std::max_element(_effective_k.begin(), _effective_k.end());

  // Calculate dN
  _dn = (unsigned long int)(_max_growth_size / (_paris_law_c * pow(_max_k, _paris_law_m)));

  _growth_size.clear();

  for (unsigned int i = 0; i < size_this_segment; ++i)
  {
    int ind = index[i];
    if (ind == -1)
      _growth_size.push_back(0.0);
    else if (ind >= 0)
    {
      Real effective_k = _effective_k[i];
      Real growth_size = _max_growth_size * pow(effective_k / _max_k, _paris_law_m);
      _growth_size.push_back(growth_size);
    }
  }

  _cutter->setSubCriticalGrowthSize(_growth_size);
}

Real
ParisLaw::getValue()
{
  return Real(_dn);
}
