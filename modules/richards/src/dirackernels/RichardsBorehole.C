#include "RichardsBorehole.h"

template<>
InputParameters validParams<RichardsBorehole>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Real>("well_constant_production", "The well constant for the borehole that will be used if fluid pressure > borehole pressure.  If this is positive then the borehole will act as a sink when fluid pressure > borehole pressure.  Set to zero if you want only injection.");
  params.addRequiredParam<Real>("well_constant_injection", "The well constant for the borehole that will be used if fluid pressure < borehole pressure.  If this is positive then the borehole will act as a source when fluid pressure < borehole pressure.  Set to zero if you want only production.");
  params.addRequiredParam<Real>("bottom_pressure", "Pressure at the bottom of the borehole");
  params.addRequiredParam<RealVectorValue>("unit_weight", "(fluid_density*gravitational_acceleration) as a vector pointing downwards.  Note that the borehole pressure at a given z position is bottom_pressure + unit_weight*(p - p_bottom), where p=(x,y,z) and p_bottom=(x,y,z) of the bottom point of the borehole.  If you don't want bottomhole pressure to vary in the borehole just set unit_weight=0.  Typical value is gravity = (0,0,-1E4)");
  params.addRequiredParam<std::string>("point_file", "The file containing the coordinates of the point sinks that approximate the borehole.  Each line in the file must contain a space-separated coordinate.  The last point in the file is defined as the borehole bottom, where the borehole pressure is bottom_pressure.  Note that you will get segementation faults if your points do not lie within your mesh!");
  params.addRequiredParam<PostprocessorName>("reporter", "The Postprocessor of type=Reporter and sum=true in which the total fluid mass flowing out of the system to the borehole for this time step will be recorded.");
  params.addClassDescription("Approximates a borehole in the mesh with given well constant using a number of point sinks whose positions are read from a file");
  return params;
}

RichardsBorehole::RichardsBorehole(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),

    _this_var_num(_var.index()),
    _p_var_nums(getMaterialProperty<std::vector<unsigned int> >("p_var_nums")),

    _well_constant_production(getParam<Real>("well_constant_production")),
    _well_constant_injection(getParam<Real>("well_constant_injection")),
    _p_bot(getParam<Real>("bottom_pressure")),
    _unit_weight(getParam<RealVectorValue>("unit_weight")),

    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),

    _dseff(getMaterialProperty<std::vector<std::vector<Real> > >("ds_eff")),

    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _drel_perm(getMaterialProperty<std::vector<Real> >("drel_perm")),

    _density(getMaterialProperty<std::vector<Real> >("density")), 
    _ddensity(getMaterialProperty<std::vector<Real> >("ddensity")),

    _reporter(getPostprocessorValue("reporter")),
    _point_file(getParam<std::string>("point_file"))

{
  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _point_file + "' from RichardsBorehole.");

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

  int num_pts = _zs.size();
  _bottom_point(0) = _xs[num_pts - 1];
  _bottom_point(1) = _ys[num_pts - 1];
  _bottom_point(2) = _zs[num_pts - 1];

}

bool RichardsBorehole::parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec)
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
RichardsBorehole::addPoints()
{
  // This function gets called just before the DiracKernel is evaluated
  // so this is a handy place to zero this out.
  _reporter = 0.0;

  for (unsigned int i = 0; i < _zs.size(); i++)
    {
      addPoint(Point(_xs[i], _ys[i], _zs[i]));
    }
}


Real
RichardsBorehole::computeQpResidual()
{
  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think

  Real mob = 0;
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  mob = _rel_perm[_qp][pvar]*_density[_qp][pvar]/_viscosity[_qp][pvar];
	}
    }

  Real flow(0.0);

  Real test_fcn = _test[_i][_qp];
  if (_u[_qp] <= bh_pressure) // injection case (borehole is a source)
    {
      flow += test_fcn*_well_constant_injection*mob*(_u[_qp] - bh_pressure);
    }
  else // production case (borehole is a sink)
    {
      flow += test_fcn*_well_constant_production*mob*(_u[_qp] - bh_pressure);
    }

  _reporter += flow*_dt; // this is not thread safe, but DiracKernel's aren't currently threaded
  return flow;
}

Real
RichardsBorehole::computeQpJacobian()
{
  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think

  Real mob(0);
  Real mobp(0);
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  mob = _rel_perm[_qp][pvar]*_density[_qp][pvar]/_viscosity[_qp][pvar];
	  mobp = (_drel_perm[_qp][pvar]*_dseff[_qp][pvar][pvar]*_density[_qp][pvar] + _rel_perm[_qp][pvar]*_ddensity[_qp][pvar])/_viscosity[_qp][pvar];
	}
    }

  Real flowp(0.0);

  Real test_fcn = _test[_i][_qp];

  if (_u[_qp] <= bh_pressure) // injection case (borehole is a source)
    {
      flowp += test_fcn*_well_constant_injection*(mob*_phi[_j][_qp] + mobp*_phi[_j][_qp]*(_u[_qp] - bh_pressure));
    }
  else // production case (borehole is a sink)
    {
      flowp += test_fcn*_well_constant_production*(mob*_phi[_j][_qp] + mobp*_phi[_j][_qp]*(_u[_qp] - bh_pressure));
    }

  return flowp;
}
