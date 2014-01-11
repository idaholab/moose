#include "RichardsBorehole.h"

template<>
InputParameters validParams<RichardsBorehole>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<UserObjectName>("porepressureNames_UO", "The UserObject that holds the list of porepressure names.");
  params.addRequiredParam<Real>("well_constant_production", "The well constant for the borehole that will be used if fluid pressure > borehole pressure.  If this is positive then the borehole will act as a sink when fluid pressure > borehole pressure.  Set to zero if you want only injection.");
  params.addRequiredParam<Real>("well_constant_injection", "The well constant for the borehole that will be used if fluid pressure < borehole pressure.  If this is positive then the borehole will act as a source when fluid pressure < borehole pressure.  Set to zero if you want only production.");
  params.addRequiredParam<Real>("bottom_pressure", "Pressure at the bottom of the borehole");
  params.addRequiredParam<RealVectorValue>("unit_weight", "(fluid_density*gravitational_acceleration) as a vector pointing downwards.  Note that the borehole pressure at a given z position is bottom_pressure + unit_weight*(p - p_bottom), where p=(x,y,z) and p_bottom=(x,y,z) of the bottom point of the borehole.  If you don't want bottomhole pressure to vary in the borehole just set unit_weight=0.  Typical value is gravity = (0,0,-1E4)");
  params.addRequiredParam<std::string>("point_file", "The file containing the coordinates of the point sinks that approximate the borehole.  Each line in the file must contain a space-separated coordinate.  The last point in the file is defined as the borehole bottom, where the borehole pressure is bottom_pressure.  Note that you will get segementation faults if your points do not lie within your mesh!");
  params.addRequiredParam<UserObjectName>("SumQuantityUO", "User Object of type=RichardsSumQuantity in which to place the total outflow from the borehole for each time step.");
  params.addClassDescription("Approximates a borehole in the mesh with given well constant using a number of point sinks whose positions are read from a file");
  return params;
}

RichardsBorehole::RichardsBorehole(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),

    _pp_name_UO(getUserObject<RichardsPorepressureNames>("porepressureNames_UO")),
    _pvar(_pp_name_UO.pressure_var_num(_var.index())),

    _well_constant_production(getParam<Real>("well_constant_production")),
    _well_constant_injection(getParam<Real>("well_constant_injection")),
    _p_bot(getParam<Real>("bottom_pressure")),
    _unit_weight(getParam<RealVectorValue>("unit_weight")),

    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),

    _permeability(getMaterialProperty<RealTensorValue>("permeability")),

    _dseff(getMaterialProperty<std::vector<std::vector<Real> > >("ds_eff")),

    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _drel_perm(getMaterialProperty<std::vector<Real> >("drel_perm")),

    _density(getMaterialProperty<std::vector<Real> >("density")), 
    _ddensity(getMaterialProperty<std::vector<Real> >("ddensity")),
  
    _total_outflow_mass(const_cast<RichardsSumQuantity &>(getUserObject<RichardsSumQuantity>("SumQuantityUO"))),
    _point_file(getParam<std::string>("point_file"))

{
  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _point_file + "' from RichardsBorehole.");

  // construct the arrays of x, array of y and array of z
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

  // construct the line-segment lengths between each point
  _seg_len.resize(num_pts-1);
  for (unsigned int i=0 ; i<_xs.size()-1; ++i)
    {
      _seg_len[i] = std::sqrt(std::pow(_xs[i+1] - _xs[i], 2) + std::pow(_ys[i+1] - _ys[i], 2) + std::pow(_zs[i+1] - _zs[i], 2));
      if (_seg_len[i] == 0)
	mooseError("Borehole has a zero-segment length at (x,y,z) = " << _xs[i] << " " << _ys[i] << " " << _zs[i] << "\n");
    }

  // construct the rotation matrix needed to rotate the permeability
  _rot_matrix.resize(num_pts-1);
  RealVectorValue x0(1,0,0);
  RealVectorValue y0(0,1,0);
  RealVectorValue z0(0,0,1);
  for (unsigned int i=0 ; i<_xs.size()-1; ++i)
    {
      // v2 is unit vector along line segment
      RealVectorValue v2(_xs[i+1] - _xs[i], _ys[i+1] - _ys[i], _zs[i+1] - _zs[i]);
      v2 /= std::sqrt(v2*v2);

      // construct v0 and v1 to be orthonormal to v2
      // TODO: perhaps there is a quicker and neater way?
      RealVectorValue v0, v1;
      Real projx = v2*x0;
      Real projy = v2*y0;
      Real projz = v2*z0;
      // possible permutations are, in order of increasing size:
      // xyz, xzy, yxz, yzx, zxy, zyx
      // for these permutations, a suitable initial direction for v0 is
      // x, x, y, y, z, z
      if ( (projz >= projy && projy >= projx) || (projy >= projz && projz >= projx) )
	v0(0) = 1;
      else if ( (projz >= projx && projx >= projy) || (projx >= projz && projz >= projy) )
	v0(1) = 1;
      else
	v0(2) = 1;
      // use cross product to get v1 = v2 x v0
      v1(0) = v2(1)*v0(2) - v2(2)*v0(1);
      v1(1) = v2(2)*v0(0) - v2(0)*v0(2);
      v1(2) = v2(0)*v0(1) - v2(1)*v0(0);
      v1 /= std::sqrt(v1*v1);
      // now use cross product to get v0 = v1 x v2
      v0(0) = v1(1)*v2(2) - v1(2)*v2(1);
      v0(1) = v1(2)*v2(0) - v1(0)*v2(2);
      v0(2) = v1(0)*v2(1) - v1(1)*v2(0);
      v0 /= std::sqrt(v0*v0); // reduces roundoff error???
	
      // rotation matrix rotates permeability tensor so that the z-prime direction lies along v2
      _rot_matrix[i](0,0) = v0*x0;
      _rot_matrix[i](0,1) = v0*y0;
      _rot_matrix[i](0,2) = v0*z0;
      _rot_matrix[i](1,0) = v1*x0;
      _rot_matrix[i](1,1) = v1*y0;
      _rot_matrix[i](1,2) = v1*z0;
      _rot_matrix[i](2,0) = v2*x0;
      _rot_matrix[i](2,1) = v2*y0;
      _rot_matrix[i](2,2) = v2*z0;
    }
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
  _total_outflow_mass.zero();

  for (unsigned int i = 0; i < _zs.size(); i++)
    {
      addPoint(Point(_xs[i], _ys[i], _zs[i]));
    }
}


Real
RichardsBorehole::computeQpResidual()
{
  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think

  Real mob = _rel_perm[_qp][_pvar]*_density[_qp][_pvar]/_viscosity[_qp][_pvar];

  RealTensorValue rot_perm = (_rot_matrix[0]*_permeability[_qp])*_rot_matrix[0].transpose();
  Real trace2D = rot_perm(0,0) + rot_perm(1,1);
  Real det2D = rot_perm(0,0)*rot_perm(1,1) - rot_perm(0,1)*rot_perm(1,0);
  Real eig_val1 = 0.5*trace2D + std::sqrt(0.25*trace2D*trace2D - det2D);
  Real eig_val2 = 0.5*trace2D - std::sqrt(0.25*trace2D*trace2D - det2D);
  RealVectorValue eig_vec1, eig_vec2;
  if (rot_perm(1,0) != 0)
    {
      eig_vec1(0) = eig_val1 - rot_perm(1,1);
      eig_vec1(1) = rot_perm(1,0);
      eig_vec2(0) = eig_val2 - rot_perm(1,1);
      eig_vec1(1) = rot_perm(1,0);
    }
  else if (rot_perm(0,1) != 0)
    {
      eig_vec1(0) = rot_perm(0,1);
      eig_vec1(1) = eig_val1 - rot_perm(0,0);
      eig_vec2(0) = rot_perm(0,1);
      eig_vec2(1) = eig_val2 - rot_perm(0,0);
    }
  else // off diagonal terms are both zero
    { 
      eig_vec1(0) = 1;
      eig_vec2(1) = 1;
    }
  // now rotate these to original frame and normalise
  eig_vec1 = _rot_matrix[0].transpose()*eig_vec1;
  eig_vec1 /= std::sqrt(eig_vec1*eig_vec1);
  eig_vec2 = _rot_matrix[0].transpose()*eig_vec2;
  eig_vec2 /= std::sqrt(eig_vec2*eig_vec2);

  Real max1 = eig_vec1*_current_elem->point(0);
  Real max2 = eig_vec2*_current_elem->point(0);
  Real min1 = max1;
  Real min2 = min2;
  Real proj;
  for (unsigned int i = 1; i < _current_elem->n_nodes(); i++)
    {
      proj = eig_vec1*_current_elem->point(i);
      max1 = (max1 < proj) ? proj : max1;
      min1 = (min1 < proj) ? min1 : proj;

      proj = eig_vec2*_current_elem->point(i);
      max2 = (max2 < proj) ? proj : max2;
      min2 = (min2 < proj) ? min2 : proj;
    }
  Real ll1 = max1 - min1;
  Real ll2 = max2 - min2;
    
  //std::cout << " max1, min1, max2, min2 " << max1 << " " << min1 << " " << max2 << " " << min2 << "\n";

  Real r0 = 0.28*std::sqrt( std::sqrt(eig_val1/eig_val2)*std::pow(ll2, 2) + std::sqrt(eig_val2/eig_val1)*std::pow(ll1, 2)) / ( std::pow(eig_val1/eig_val2, 0.25) + std::pow(eig_val2/eig_val1, 0.25) );
  
  Real effective_perm = std::sqrt(det2D);
  //std::cout << "eff = " << effective_perm << " rot_perm=" << rot_perm << "\n";

  Real bhole_len = 1.0;
  Real mypi = 3.1415927;
  Real borehole_radius = 1.0;

  Real wc = 2*mypi*effective_perm*bhole_len/std::log(r0/borehole_radius);

  std::cout << "wc = " << wc << "\n";

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

  _total_outflow_mass.add(flow*_dt); // this is not thread safe, but DiracKernel's aren't currently threaded
  return flow;
}

Real
RichardsBorehole::computeQpJacobian()
{
  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think

  Real mob = _rel_perm[_qp][_pvar]*_density[_qp][_pvar]/_viscosity[_qp][_pvar];
  Real mobp = (_drel_perm[_qp][_pvar]*_dseff[_qp][_pvar][_pvar]*_density[_qp][_pvar] + _rel_perm[_qp][_pvar]*_ddensity[_qp][_pvar])/_viscosity[_qp][_pvar];

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
