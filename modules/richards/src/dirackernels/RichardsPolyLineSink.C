#include "RichardsPolyLineSink.h"

template<>
InputParameters validParams<RichardsPolyLineSink>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<std::vector<Real> >("pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real> >("fluxes", "Tuple of flux values (measured in kg.m^-3.s^-1).  A piecewise-linear fit is performed to the (pressure,flux) pairs to obtain the flux at any arbitrary pressure.  If a quad-point pressure is less than the first pressure value, the first flux value is used.  If quad-point pressure exceeds the final pressure value, the final flux value is used.  This flux is OUT of the medium: hence positive values of flux means this will be a SINK, while negative values indicate this flux will be a SOURCE.");
  params.addRequiredParam<std::string>("point_file", "The file containing the coordinates of the point sinks that will approximate the polyline.  Each line in the file must contain a space-separated coordinate.  Note that you will get segementation faults if your points do not lie within your mesh!");
  params.addRequiredParam<PostprocessorName>("reporter", "The Postprocessor of type=Reporter and sum=true in which the total fluid mass flowing out of the system to the sink for this time step will be recorded.");
  params.addClassDescription("Approximates a polyline sink in the mesh by using a number of point sinks whose positions are read from a file");
  return params;
}

RichardsPolyLineSink::RichardsPolyLineSink(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),
    _reporter(getPostprocessorValue("reporter")),
    _sink_func(getParam<std::vector<Real> >("pressures"), getParam<std::vector<Real> >("fluxes")),
    _point_file(getParam<std::string>("point_file")),
    _vel_SUPG(getMaterialProperty<RealVectorValue>("vel_SUPG")),
    _vel_prime_SUPG(getMaterialProperty<RealTensorValue>("vel_prime_SUPG")),
    _tau_SUPG(getMaterialProperty<Real>("tau_SUPG")),
    _tau_prime_SUPG(getMaterialProperty<RealVectorValue>("tau_prime_SUPG"))
{
  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _point_file + "' from RichardsPolyLineSink.");

  std::vector<Real> scratch;
  while (parseNextLineReals(file, scratch))
    {
      if (scratch.size() >= 1){
	_xs.push_back(scratch[0]);
	if (scratch.size() >= 2){
	  _ys.push_back(scratch[1]);
	}
	else{
	  _ys.push_back(0.0);
	}
	if (scratch.size() >= 3){
	  _zs.push_back(scratch[2]);
	}
	else{
	  _zs.push_back(0.0);
	}
      }
    }

  file.close();
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
  _reporter = 0.0;
  for (int i = 0; i < _zs.size(); i++)
    {
      addPoint(Point(_xs[i], _ys[i], _zs[i]));
    }
}


Real
RichardsPolyLineSink::computeQpResidual()
{
  Real test_fcn = _test[_i][_qp] + _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp]*0;
  Real flow = test_fcn*_sink_func.sample(_u[_qp]);
  _reporter += flow*_dt; // this is not thread safe, but DiracKernel's aren't currently threaded
  return flow;
}

Real
RichardsPolyLineSink::computeQpJacobian()
{
  Real test_fcn = _test[_i][_qp] + _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp]*0;
  Real supg_test_prime = (_tau_prime_SUPG[_qp]*_grad_phi[_j][_qp])*(_vel_SUPG[_qp]*_grad_test[_i][_qp]) + _tau_SUPG[_qp]*(_vel_prime_SUPG[_qp]*_grad_phi[_j][_qp])*_grad_test[_i][_qp];
  return test_fcn*_sink_func.deriv(_u[_qp])*_phi[_j][_qp] + supg_test_prime*_sink_func.sample(_u[_qp])*0;
}
