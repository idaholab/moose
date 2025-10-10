//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackGrowthReporterBase.h"
#include "CrackMeshCut3DUserObject.h"
#include "VectorPostprocessorInterface.h"
#include "MathUtils.h"

InputParameters
CrackGrowthReporterBase::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<UserObjectName>("crackMeshCut3DUserObject_name",
                                          "The CrackMeshCut3DUserObject user object name");
  params.addRequiredParam<Real>(
      "max_growth_size",
      "the max growth size at the crack front in each increment of a fatigue simulation");
  params.addParam<VectorPostprocessorName>(
      "ki_vectorpostprocessor", "II_KI_1", "The name of the vectorpostprocessor that contains KI");

  // these flags must must match those used by the InteractionIntegral set in the
  // DomainIntegralAction
  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_XFEM_MARK);
  params.setDocString("execute_on", exec.getDocString());
  params.set<ExecFlagEnum>("execute_on") = {EXEC_XFEM_MARK, EXEC_TIMESTEP_END};
  return params;
}

CrackGrowthReporterBase::CrackGrowthReporterBase(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _cutter_name(getParam<UserObjectName>("crackMeshCut3DUserObject_name")),
    _3Dcutter(&_fe_problem.getUserObject<CrackMeshCut3DUserObject>(_cutter_name)),
    _max_growth_size(getParam<Real>("max_growth_size")),
    _ki_vpp(getVectorPostprocessorValue(
        "ki_vectorpostprocessor", getParam<VectorPostprocessorName>("ki_vectorpostprocessor"))),
    _ki_x(getVectorPostprocessorValue("ki_vectorpostprocessor", "x")),
    _ki_y(getVectorPostprocessorValue("ki_vectorpostprocessor", "y")),
    _ki_z(getVectorPostprocessorValue("ki_vectorpostprocessor", "z")),
    _ki_id(getVectorPostprocessorValue("ki_vectorpostprocessor", "id")),
    _x(declareValueByName<std::vector<Real>>("x", REPORTER_MODE_ROOT)),
    _y(declareValueByName<std::vector<Real>>("y", REPORTER_MODE_ROOT)),
    _z(declareValueByName<std::vector<Real>>("z", REPORTER_MODE_ROOT)),
    _id(declareValueByName<std::vector<Real>>("id", REPORTER_MODE_ROOT))
{
}

void
CrackGrowthReporterBase::execute()
{
  copy_coordinates();
  std::vector<int> index = get_cutter_mesh_indices();
  compute_growth(index);
}

void
CrackGrowthReporterBase::copy_coordinates() const
{
  _x.resize(_ki_x.size());
  _y.resize(_ki_y.size());
  _z.resize(_ki_z.size());
  _id.resize(_ki_id.size());
  std::copy(_ki_x.begin(), _ki_x.end(), _x.begin());
  std::copy(_ki_y.begin(), _ki_y.end(), _y.begin());
  std::copy(_ki_z.begin(), _ki_z.end(), _z.begin());
  std::copy(_ki_id.begin(), _ki_id.end(), _id.begin());
}

std::vector<int>
CrackGrowthReporterBase::get_cutter_mesh_indices() const
{
  // Generate _active_boundary and _inactive_boundary_pos;
  // This is a duplicated call before the one in CrackMeshCut3DUserObject;
  // That one cannot be deleted because this one is for subcritical cracking only
  // the test still pass so lynn is not sure if this is actually needed but it was
  // in the original post processor so I'll keep it, even though it is non const.
  _3Dcutter->findActiveBoundaryNodes();
  return _3Dcutter->getFrontPointsIndex();
}
