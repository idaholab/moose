//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsPolyLineSink.h"

#include <fstream>

registerMooseObject("RichardsApp", RichardsPolyLineSink);

InputParameters
RichardsPolyLineSink::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addRequiredParam<std::vector<Real>>(
      "pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real>>(
      "fluxes",
      "Tuple of flux values (measured in kg.m^-3.s^-1).  A piecewise-linear fit is "
      "performed to the (pressure,flux) pairs to obtain the flux at any arbitrary "
      "pressure.  If a quad-point pressure is less than the first pressure value, the "
      "first flux value is used.  If quad-point pressure exceeds the final pressure "
      "value, the final flux value is used.  This flux is OUT of the medium: hence "
      "positive values of flux means this will be a SINK, while negative values indicate "
      "this flux will be a SOURCE.");
  params.addRequiredParam<FileName>(
      "point_file",
      "The file containing the coordinates of the point sinks that will approximate "
      "the polyline.  Each line in the file must contain a space-separated "
      "coordinate.  Note that you will get segementation faults if your points do "
      "not lie within your mesh!");
  params.addRequiredParam<UserObjectName>(
      "SumQuantityUO",
      "User Object of type=RichardsSumQuantity in which to place the total "
      "outflow from the polylinesink for each time step.");
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addClassDescription("Approximates a polyline sink in the mesh by using a number of point "
                             "sinks whose positions are read from a file");
  return params;
}

RichardsPolyLineSink::RichardsPolyLineSink(const InputParameters & parameters)
  : DiracKernel(parameters),
    _total_outflow_mass(
        const_cast<RichardsSumQuantity &>(getUserObject<RichardsSumQuantity>("SumQuantityUO"))),
    _sink_func(getParam<std::vector<Real>>("pressures"), getParam<std::vector<Real>>("fluxes")),
    _point_file(getParam<FileName>("point_file")),
    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),
    _pp(getMaterialProperty<std::vector<Real>>("porepressure")),
    _dpp_dv(getMaterialProperty<std::vector<std::vector<Real>>>("dporepressure_dv"))
{
  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _point_file + "' from RichardsPolyLineSink.");

  std::vector<Real> scratch;
  while (parseNextLineReals(file, scratch))
  {
    if (scratch.size() >= 1)
    {
      _xs.push_back(scratch[0]);
      if (scratch.size() >= 2)
        _ys.push_back(scratch[1]);
      else
        _ys.push_back(0.0);

      if (scratch.size() >= 3)
        _zs.push_back(scratch[2]);
      else
        _zs.push_back(0.0);
    }
  }

  file.close();

  // To correctly compute the Jacobian terms,
  // tell MOOSE that this DiracKernel depends on all the Richards Vars
  const std::vector<MooseVariableFEBase *> & coupled_vars = _richards_name_UO.getCoupledMooseVars();
  for (unsigned int i = 0; i < coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}

bool
RichardsPolyLineSink::parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec)
// reads a space-separated line of floats from ifs and puts in myvec
{
  std::string line;
  myvec.clear();
  bool gotline(false);
  if (getline(ifs, line))
  {
    gotline = true;

    // Harvest floats separated by whitespace
    std::istringstream iss(line);
    Real f;
    while (iss >> f)
    {
      myvec.push_back(f);
    }
  }
  return gotline;
}

void
RichardsPolyLineSink::addPoints()
{
  _total_outflow_mass.zero();

  // Add point using the unique ID "i", let the DiracKernel take
  // care of the caching.  This should be fast after the first call,
  // as long as the points don't move around.
  for (unsigned int i = 0; i < _zs.size(); i++)
    addPoint(Point(_xs[i], _ys[i], _zs[i]), i);
}

Real
RichardsPolyLineSink::computeQpResidual()
{
  Real test_fcn = _test[_i][_qp];
  Real flow = test_fcn * _sink_func.sample(_pp[_qp][_pvar]);
  _total_outflow_mass.add(flow * _dt);
  return flow;
}

Real
RichardsPolyLineSink::computeQpJacobian()
{
  Real test_fcn = _test[_i][_qp];
  return test_fcn * _sink_func.sampleDerivative(_pp[_qp][_pvar]) * _dpp_dv[_qp][_pvar][_pvar] *
         _phi[_j][_qp];
}

Real
RichardsPolyLineSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;
  unsigned int dvar = _richards_name_UO.richards_var_num(jvar);
  Real test_fcn = _test[_i][_qp];
  return test_fcn * _sink_func.sampleDerivative(_pp[_qp][_pvar]) * _dpp_dv[_qp][_pvar][dvar] *
         _phi[_j][_qp];
}
