//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParisLaw.h"
#include "CrackMeshCut3DUserObject.h"
#include "VectorPostprocessorInterface.h"
#include "MathUtils.h"

registerMooseObject("XFEMApp", ParisLaw);

InputParameters
ParisLaw::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "This reporter computes the crack extension size at all active crack front points "
      "in the CrackMeshCut3DUserObject.  This reporter is in the same order as the fracture "
      "integral vpps it gets data from.");
  params.addRequiredParam<UserObjectName>("crackMeshCut3DUserObject_name",
                                          "The CrackMeshCut3DUserObject user object name");
  params.addRequiredParam<Real>(
      "max_growth_size",
      "the max growth size at the crack front in each increment of a fatigue simulation");
  params.addRequiredParam<Real>("paris_law_c", "parameter C in the Paris law for fatigue");
  params.addRequiredParam<Real>("paris_law_m", "parameter m in the Paris law for fatigue");
  params.addParam<VectorPostprocessorName>(
      "ki_vectorpostprocessor", "II_KI_1", "The name of the vectorpostprocessor that contains KI");
  params.addParam<VectorPostprocessorName>("kii_vectorpostprocessor",
                                           "II_KII_1",
                                           "The name of the vectorpostprocessor that contains KII");
  params.addParam<ReporterValueName>(
      "growth_increment_name",
      "growth_increment",
      "Reporter name containing growth increments for the crack front points.");
  params.addParam<ReporterValueName>(
      "cycles_to_max_growth_size_name",
      "dN",
      "Reporter name containing the number of cycles to reach max_growth_size.");

  // these flags must must match those used by the InteractionIntegral set in the
  // DomainIntegralAction
  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_XFEM_MARK);
  params.setDocString("execute_on", exec.getDocString());
  params.set<ExecFlagEnum>("execute_on") = {EXEC_XFEM_MARK, EXEC_TIMESTEP_END};
  return params;
}

ParisLaw::ParisLaw(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _cutter_name(getParam<UserObjectName>("crackMeshCut3DUserObject_name")),
    _3Dcutter(&_fe_problem.getUserObject<CrackMeshCut3DUserObject>(_cutter_name)),
    _max_growth_size(getParam<Real>("max_growth_size")),
    _paris_law_c(getParam<Real>("paris_law_c")),
    _paris_law_m(getParam<Real>("paris_law_m")),
    _ki_vpp(getVectorPostprocessorValue(
        "ki_vectorpostprocessor", getParam<VectorPostprocessorName>("ki_vectorpostprocessor"))),
    _kii_vpp(getVectorPostprocessorValue(
        "kii_vectorpostprocessor", getParam<VectorPostprocessorName>("kii_vectorpostprocessor"))),
    _ki_x(getVectorPostprocessorValue("ki_vectorpostprocessor", "x")),
    _ki_y(getVectorPostprocessorValue("ki_vectorpostprocessor", "y")),
    _ki_z(getVectorPostprocessorValue("ki_vectorpostprocessor", "z")),
    _ki_id(getVectorPostprocessorValue("ki_vectorpostprocessor", "id")),
    _dn(declareValueByName<Real>(getParam<ReporterValueName>("cycles_to_max_growth_size_name"),
                                 REPORTER_MODE_ROOT)),
    _growth_increment(declareValueByName<std::vector<Real>>(
        getParam<ReporterValueName>("growth_increment_name"), REPORTER_MODE_ROOT)),
    _x(declareValueByName<std::vector<Real>>("x", REPORTER_MODE_ROOT)),
    _y(declareValueByName<std::vector<Real>>("y", REPORTER_MODE_ROOT)),
    _z(declareValueByName<std::vector<Real>>("z", REPORTER_MODE_ROOT)),
    _id(declareValueByName<std::vector<Real>>("id", REPORTER_MODE_ROOT))
{
}

void
ParisLaw::execute()
{
  _x.resize(_ki_x.size());
  _y.resize(_ki_y.size());
  _z.resize(_ki_z.size());
  _id.resize(_ki_id.size());
  std::copy(_ki_x.begin(), _ki_x.end(), _x.begin());
  std::copy(_ki_y.begin(), _ki_y.end(), _y.begin());
  std::copy(_ki_z.begin(), _ki_z.end(), _z.begin());
  std::copy(_ki_id.begin(), _ki_id.end(), _id.begin());

  _growth_increment.resize(_ki_x.size());

  // Generate _active_boundary and _inactive_boundary_pos;
  // This is a duplicated call before the one in CrackMeshCut3DUserObject;
  // That one cannot be deleted because this one is for subcritical cracking only
  // the test still pass so lynn is not sure if this is actually needed but it was
  // in the original post processor so I'll keep it, even though it is non const.
  _3Dcutter->findActiveBoundaryNodes();
  std::vector<int> index = _3Dcutter->getFrontPointsIndex();

  std::vector<Real> effective_k(_ki_x.size());
  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    int ind = index[i];
    if (ind == -1)
      effective_k[i] = 0;
    else
      effective_k[i] =
          std::sqrt(Utility::pow<2>(_ki_vpp[ind]) + 2 * Utility::pow<2>(_kii_vpp[ind]));
  }

  Real _max_k = *std::max_element(effective_k.begin(), effective_k.end());
  _dn = _max_growth_size / (_paris_law_c * std::pow(_max_k, _paris_law_m));

  for (std::size_t i = 0; i < _ki_vpp.size(); ++i)
  {
    int ind = index[i];
    if (ind == -1)
      _growth_increment[ind] = 0.0;
    else
      _growth_increment[ind] = _max_growth_size * std::pow(effective_k[i] / _max_k, _paris_law_m);
  }
}
