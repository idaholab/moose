/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsPolyLineSink.h"

template<>
InputParameters validParams<RichardsPolyLineSink>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<std::vector<Real> >("pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("fluxes", "Tuple of flux values (measured in kg.m^-3.s^-1).  A piecewise-linear fit is performed to the (pressure,flux) pairs to obtain the flux at any arbitrary pressure.  If a quad-point pressure is less than the first pressure value, the first flux value is used.  If quad-point pressure exceeds the final pressure value, the final flux value is used.  This flux is OUT of the medium: hence positive values of flux means this will be a SINK, while negative values indicate this flux will be a SOURCE.");
  params.addRequiredParam<std::string>("point_file", "The file containing the coordinates of the point sinks that will approximate the polyline.  Each line in the file must contain a space-separated coordinate.  Note that you will get segementation faults if your points do not lie within your mesh!");
  params.addParam<bool>("mesh_adaptivity", true, "If not using mesh adaptivity then set this false to substantially speed up the simulation by caching the element containing each Dirac point.");
  params.addRequiredParam<UserObjectName>("SumQuantityUO", "User Object of type=RichardsSumQuantity in which to place the total outflow from the polylinesink for each time step.");
  params.addClassDescription("Approximates a polyline sink in the mesh by using a number of point sinks whose positions are read from a file");
  return params;
}

RichardsPolyLineSink::RichardsPolyLineSink(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),
    _total_outflow_mass(const_cast<RichardsSumQuantity &>(getUserObject<RichardsSumQuantity>("SumQuantityUO"))),
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("fluxes")),
    _point_file(getParam<std::string>("point_file")),
    _mesh_adaptivity(getParam<bool>("mesh_adaptivity"))
{
  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _point_file + "' from RichardsPolyLineSink.");

  std::vector<Real> scratch;
  while (parseNextLineReals(file, scratch))
  {
    if (scratch.size() >= 1) {
      _xs.push_back(scratch[0]);
      if (scratch.size() >= 2) {
        _ys.push_back(scratch[1]);
      }
      else {
        _ys.push_back(0.0);
      }
      if (scratch.size() >= 3) {
        _zs.push_back(scratch[2]);
      }
      else {
        _zs.push_back(0.0);
      }
    }
  }

  file.close();

  // size the array that holds elemental info
  int num_pts = _zs.size();
  _elemental_info.resize(num_pts);
  _have_constructed_elemental_info = false;

}

bool RichardsPolyLineSink::parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec)
// reads a space-separated line of floats from ifs and puts in myvec
{
  std::string line;
  myvec.clear();
  bool gotline(false);
  if (getline(ifs,line))
  {
    gotline=true;

    //Harvest floats separated by whitespace
    std::istringstream iss(line);
    Real f;
    while (iss>>f)
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

  if (!_have_constructed_elemental_info || _mesh_adaptivity)
  {
    for (unsigned int i = 0; i < _zs.size(); i++)
      _elemental_info[i] = addPoint(Point(_xs[i], _ys[i], _zs[i]));
    _have_constructed_elemental_info = true;
  }
  else
  {
    for (unsigned int i = 0; i < _zs.size(); i++)
      addPoint(_elemental_info[i], Point(_xs[i], _ys[i], _zs[i]));
  }
}


Real
RichardsPolyLineSink::computeQpResidual()
{
  Real test_fcn = _test[_i][_qp];
  Real flow = test_fcn*_sink_func.sample(_u[_qp]);
  _total_outflow_mass.add(flow*_dt);
  return flow;
}

Real
RichardsPolyLineSink::computeQpJacobian()
{
  Real test_fcn = _test[_i][_qp];
  return test_fcn*_sink_func.sampleDerivative(_u[_qp])*_phi[_j][_qp];
}
