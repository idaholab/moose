//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "WaveSpeedVPP.h"
#include "ElementsIntersectedByPlane.h"
#include "MooseMesh.h"
#include "HLLCUserObject.h"

registerMooseObject("NavierStokesApp", WaveSpeedVPP);

InputParameters
WaveSpeedVPP::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addRequiredParam<unsigned int>("elem_id", "ID of the element");
  params.addRequiredParam<unsigned int>("side_id", "ID of the side");
  params.addRequiredParam<UserObjectName>("hllc_uo", "Name of HLLC UO");
  params.addClassDescription("Extracts wave speeds from HLLC userobject for a given face");
  return params;
}

WaveSpeedVPP::WaveSpeedVPP(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _hllc(getUserObject<HLLCUserObject>("hllc_uo")),
    _side_id(getParam<unsigned int>("side_id")),
    _wave_speeds(declareVector("wave_speed"))
{
}

void
WaveSpeedVPP::initialize()
{
  _elem = _fe_problem.mesh().queryElemPtr(getParam<unsigned int>("elem_id"));
  _wave_speeds.clear();
}

void
WaveSpeedVPP::execute()
{
  if (!_elem || !_hllc.hasData(_elem, _side_id))
    return;

  const auto & ws = _hllc.waveSpeed(_elem, _side_id);
  _wave_speeds.resize(ws.size());
  for (const auto j : index_range(ws))
    _wave_speeds[j] = MetaPhysicL::raw_value(ws[j]);
}

void
WaveSpeedVPP::finalize()
{
  _communicator.allgather(_wave_speeds);
}
