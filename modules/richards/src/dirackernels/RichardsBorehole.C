/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsBorehole.h"
#include "RotationMatrix.h"

template<>
InputParameters validParams<RichardsBorehole>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<UserObjectName>("richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addRequiredParam<FunctionName>("character", "If zero then borehole does nothing.  If positive the borehole acts as a sink (production well) for porepressure > borehole pressure, and does nothing otherwise.  If negative the borehole acts as a source (injection well) for porepressure < borehole pressure, and does nothing otherwise.  The flow rate to/from the borehole is multiplied by |character|, so usually character = +/- 1, but you can specify other quantities to provide an overall scaling to the flow if you like.");
  params.addRequiredParam<Real>("bottom_pressure", "Pressure at the bottom of the borehole");
  params.addRequiredParam<RealVectorValue>("unit_weight", "(fluid_density*gravitational_acceleration) as a vector pointing downwards.  Note that the borehole pressure at a given z position is bottom_pressure + unit_weight*(p - p_bottom), where p=(x,y,z) and p_bottom=(x,y,z) of the bottom point of the borehole.  If you don't want bottomhole pressure to vary in the borehole just set unit_weight=0.  Typical value is gravity = (0,0,-1E4)");
  params.addRequiredParam<std::string>("point_file", "The file containing the borehole radii and coordinates of the point sinks that approximate the borehole.  Each line in the file must contain a space-separated radius and coordinate.  Ie r x y z.  The last point in the file is defined as the borehole bottom, where the borehole pressure is bottom_pressure.  Note that you will get segementation faults if your points do not lie within your mesh!");
  params.addRequiredParam<UserObjectName>("SumQuantityUO", "User Object of type=RichardsSumQuantity in which to place the total outflow from the borehole for each time step.");
  params.addParam<Real>("re_constant", 0.28, "The dimensionless constant used in evaluating the borehole effective radius.  This depends on the meshing scheme.  Peacemann finite-difference calculations give 0.28, while for rectangular finite elements the result is closer to 0.1594.  (See  Eqn(4.13) of Z Chen, Y Zhang, Well flow models for various numerical methods, Int J Num Analysis and Modeling, 3 (2008) 375-388.)");
  params.addParam<bool>("mesh_adaptivity", true, "If not using mesh adaptivity then set this false to substantially speed up the simulation by caching the element containing each Dirac point.");
  params.addParam<Real>("well_constant", -1.0, "Usually this is calculated internally from the element geometry, the local borehole direction and segment length, and the permeability.  However, if this parameter is given as a positive number then this number is used instead of the internal calculation.  This speeds up computation marginally.  re_constant becomes irrelevant");
  params.addCoupledVar("dseff_var", 0.0, "Derivative of Seff wrt the pressure variable.  Usually this is calculated by RichardsMaterial: inputting this parameter allows the user greater freedom");
  params.addCoupledVar("relperm_var", 0.0, "Relative permeability.  Usually this is calculated by RichardsMaterial: inputting this parameter allows the user greater freedom");
  params.addCoupledVar("drelperm_var", 0.0, "Derivative of relative permeability wrt effective saturation.  Usually this is calculated by RichardsMaterial: inputting this parameter allows the user greater freedom");
  params.addCoupledVar("density_var", 0.0, "Fluid density.  Usually this is calculated by RichardsMaterial: inputting this parameter allows the user greater freedom");
  params.addCoupledVar("ddensity_var", 0.0, "Derivative of fluid density wrt pressure.  Usually this is calculated by RichardsMaterial: inputting this parameter allows the user greater freedom");
  params.addParam<bool>("MyNameIsAndyWilkins", false, "Used for debugging by Andy");
  params.addClassDescription("Approximates a borehole in the mesh with given bottomhole pressure, and radii using a number of point sinks whose positions are read from a file");
  return params;
}

RichardsBorehole::RichardsBorehole(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),

    _debug_things(getParam<bool>("MyNameIsAndyWilkins")),

    _dseff_val(&coupledValue("dseff_var")),
    _relperm_val(&coupledValue("relperm_var")),
    _drelperm_val(&coupledValue("drelperm_var")),
    _density_val(&coupledValue("density_var")),
    _ddensity_val(&coupledValue("ddensity_var")),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    _character(getFunction("character")),
    _p_bot(getParam<Real>("bottom_pressure")),
    _unit_weight(getParam<RealVectorValue>("unit_weight")),

    _re_constant(getParam<Real>("re_constant")),
    _well_constant(getParam<Real>("well_constant")),

    _mesh_adaptivity(getParam<bool>("mesh_adaptivity")),

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
  // zero the outflow mass
  _total_outflow_mass.zero();

  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _point_file + "' from RichardsBorehole.");

  // construct the arrays of radius, x, y and z
  std::vector<Real> scratch;
  while (parseNextLineReals(file, scratch))
  {
    if (scratch.size() >= 2)
    {
      _rs.push_back(scratch[0]);
      _xs.push_back(scratch[1]);
      if (scratch.size() >= 3)
        _ys.push_back(scratch[2]);
      else
        _ys.push_back(0.0);
      if (scratch.size() >= 4)
        _zs.push_back(scratch[3]);
      else
        _zs.push_back(0.0);
    }
  }

  file.close();

  int num_pts = _zs.size();
  _bottom_point(0) = _xs[num_pts - 1];
  _bottom_point(1) = _ys[num_pts - 1];
  _bottom_point(2) = _zs[num_pts - 1];

  // construct the line-segment lengths between each point
  _half_seg_len.resize(num_pts-1);
  for (unsigned int i=0 ; i<_xs.size()-1; ++i)
  {
    _half_seg_len[i] = 0.5*std::sqrt(std::pow(_xs[i+1] - _xs[i], 2) + std::pow(_ys[i+1] - _ys[i], 2) + std::pow(_zs[i+1] - _zs[i], 2));
    if (_half_seg_len[i] == 0)
      mooseError("Borehole has a zero-segment length at (x,y,z) = " << _xs[i] << " " << _ys[i] << " " << _zs[i] << "\n");
  }

  // construct the rotation matrix needed to rotate the permeability
  _rot_matrix.resize(num_pts-1);
  for (unsigned int i=0 ; i<_xs.size()-1; ++i)
  {
    RealVectorValue v2(_xs[i+1] - _xs[i], _ys[i+1] - _ys[i], _zs[i+1] - _zs[i]);
    _rot_matrix[i] = RotationMatrix::rotVecToZ(v2);
  }

  // size the array that holds elemental info
  _elemental_info.resize(num_pts);
  _have_constructed_elemental_info = false;

  // do debugging if AndyWilkins
  if (_debug_things)
  {
    Moose::out << "Checking rotation matrices\n";
    RealVectorValue zzz(0,0,1);
    RealTensorValue iii;
    iii(0,0) = 1;
    iii(1,1) = 1;
    iii(2,2) = 1;
    RealVectorValue vec0;
    RealTensorValue ten0;
    Real the_sum;
    for (unsigned int i=0 ; i<_xs.size()-1; ++i)
    {
      // check rotation matrix does the correct rotation
      Moose::out << i << "\n";
      RealVectorValue v2(_xs[i+1] - _xs[i], _ys[i+1] - _ys[i], _zs[i+1] - _zs[i]);
      v2 /= std::sqrt(v2*v2);
      vec0 = _rot_matrix[i]*v2 - zzz;
      if ((vec0*vec0) > 1E-20)
        mooseError("Rotation matrix for v2 = " << v2 << " is wrong.  It is " << _rot_matrix[i] << "\n");

      // check rotation matrix is orthogonal
      ten0 = _rot_matrix[i]*_rot_matrix[i].transpose() - iii;
      the_sum = 0;
      for (unsigned int j=0 ; j<3; ++j)
        for (unsigned int k=0 ; k<3; ++k)
          the_sum = ten0(j,k)*ten0(j,k);
      if (the_sum > 1E-20)
        mooseError("Rotation matrix for v2 = " << v2 << " does not obey R.R^T=I.  It is " << _rot_matrix[i] << "\n");

      ten0 = _rot_matrix[i].transpose()*_rot_matrix[i] - iii;
      the_sum = 0;
      for (unsigned int j=0 ; j<3; ++j)
        for (unsigned int k=0 ; k<3; ++k)
          the_sum = ten0(j,k)*ten0(j,k);
      if (the_sum > 1E-20)
        mooseError("Rotation matrix for v2 = " << v2 << " does not obey R^T.R=I.  It is " << _rot_matrix[i] << "\n");
    }
  }

  // while it should be possible to have, say, relperm_var set, but drelperm_var not set,
  // i feel this could lead to hard-to-debug errors in Jacobian calculations.  Hence the following:
  if (isCoupled("dseff_var") && isCoupled("relperm_var") && isCoupled("drelperm_var") && isCoupled("density_var") && isCoupled("ddensity_var"))
    _using_coupled_vars = true;
  else if (!(isCoupled("dseff_var") && isCoupled("relperm_var") && isCoupled("drelperm_var") && isCoupled("density_var") && isCoupled("ddensity_var")))
    _using_coupled_vars = false;
  else
    mooseError("To reduce errors, if you are using one CoupledVar in a RichardsBorehole (for example, the relperm_var), you must currently use ALL of the CoupledVars (dseff_var, relperm_var, drelperm_var, etc)");

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

  if (!_have_constructed_elemental_info || _mesh_adaptivity)
  {
    for (unsigned int i = 0; i < _zs.size(); i++)
    {
      _elemental_info[i] = addPoint(Point(_xs[i], _ys[i], _zs[i]));
      if (!_elemental_info[i])
        mooseError("RichardsBorehole: the point " << _xs[i] << " " << _ys[i] << " " << _zs[i] << " was not added since it is not in any element");
    }
    _have_constructed_elemental_info = true;
  }
  else
  {
    for (unsigned int i = 0; i < _zs.size(); i++)
      addPoint(_elemental_info[i], Point(_xs[i], _ys[i], _zs[i]));
  }

}

Real
RichardsBorehole::wellConstant(const RealTensorValue & perm, const RealTensorValue & rot, const Real & half_len, const Elem * ele, const Real & rad)
// Peaceman's form for the borehole well constant
{
  if (_well_constant > 0)
    return _well_constant;

  // rot_perm has its "2" component lying along the half segment
  // we want to determine the eigenvectors of rot(0:1, 0:1), since, when
  // rotated back to the original frame we will determine the element
  // lengths along these directions
  RealTensorValue rot_perm = (rot*perm)*rot.transpose();
  Real trace2D = rot_perm(0,0) + rot_perm(1,1);
  Real det2D = rot_perm(0,0)*rot_perm(1,1) - rot_perm(0,1)*rot_perm(1,0);
  Real sq = std::sqrt(std::max(0.25*trace2D*trace2D - det2D, 0.0)); // the std::max accounts for wierdo precision loss
  Real eig_val1 = 0.5*trace2D + sq;
  Real eig_val2 = 0.5*trace2D - sq;
  RealVectorValue eig_vec1, eig_vec2;
  if (sq > std::abs(trace2D)*1E-7) // matrix is not a multiple of the identity (1E-7 accounts for precision in a crude way)
  {
    if (rot_perm(1,0) != 0)
    {
      eig_vec1(0) = eig_val1 - rot_perm(1,1);
      eig_vec1(1) = rot_perm(1,0);
      eig_vec2(0) = eig_val2 - rot_perm(1,1);
      eig_vec2(1) = rot_perm(1,0);
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
  }
  else // matrix is basically a multiple of the identity
  {
    eig_vec1(0) = 1;
    eig_vec2(1) = 1;
  }


  // finally, rotate these to original frame and normalise
  eig_vec1 = rot.transpose()*eig_vec1;
  eig_vec1 /= std::sqrt(eig_vec1*eig_vec1);
  eig_vec2 = rot.transpose()*eig_vec2;
  eig_vec2 /= std::sqrt(eig_vec2*eig_vec2);

  // find the "length" of the element in these directions
  // TODO - probably better to use variance than max&min
  Real max1 = eig_vec1*ele->point(0);
  Real max2 = eig_vec2*ele->point(0);
  Real min1 = max1;
  Real min2 = max2;
  Real proj;
  for (unsigned int i = 1; i < ele->n_nodes(); i++)
  {
    proj = eig_vec1*ele->point(i);
    max1 = (max1 < proj) ? proj : max1;
    min1 = (min1 < proj) ? min1 : proj;

    proj = eig_vec2*ele->point(i);
    max2 = (max2 < proj) ? proj : max2;
    min2 = (min2 < proj) ? min2 : proj;
  }
  Real ll1 = max1 - min1;
  Real ll2 = max2 - min2;

  //Moose::out << " max1, min1, max2, min2 " << max1 << " " << min1 << " " << max2 << " " << min2 << "\n";

  Real r0 = _re_constant*std::sqrt( std::sqrt(eig_val1/eig_val2)*std::pow(ll2, 2) + std::sqrt(eig_val2/eig_val1)*std::pow(ll1, 2)) / ( std::pow(eig_val1/eig_val2, 0.25) + std::pow(eig_val2/eig_val1, 0.25) );

  Real effective_perm = std::sqrt(det2D);
  //Moose::out << "eff = " << effective_perm << " rot_perm=" << rot_perm << "\n";

  const Real halfPi = acos(0.0);

  if (r0 <= rad)
    mooseError("The effective element size (about 0.2-times-true-ele-size) for an element containing a RichardsBorehole must be (much) larger than the borehole radius for the Peaceman formulation to be correct.  Your element has effective size " << r0 << " and the borehole radius is " << rad << "\n");

  //Moose::out << "half_len = " << half_len << " r0 = " << r0 << " rad = " << rad << "\n";

  //Moose::out << "computed wc= " << 4*halfPi*effective_perm*half_len/std::log(r0/rad) << "\n";
  return 4*halfPi*effective_perm*half_len/std::log(r0/rad);
}

Real
RichardsBorehole::computeQpResidual()
{
  Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0) return 0.0;

  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think?!

  Real mob = 1.0/_viscosity[_qp][_pvar];
  mob *= (_using_coupled_vars ? (*_relperm_val)[_qp]*(*_density_val)[_qp] : _rel_perm[_qp][_pvar]*_density[_qp][_pvar]);


  // when we can query for the current_dirac_ptid replace this complete bodge:
  unsigned int current_dirac_ptid = 0;
  if (_zs.size() > 2)
    current_dirac_ptid = 1;

  Real outflow(0.0); // this is the flow rate from porespace out of the system

  Real wc(0.0);
  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point
  {
    wc = wellConstant(_permeability[_qp], _rot_matrix[current_dirac_ptid - 1], _half_seg_len[current_dirac_ptid - 1], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _u[_qp] < bh_pressure) || (character > 0.0 && _u[_qp] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflow += _test[_i][_qp]*std::abs(character)*wc*mob*(_u[_qp] - bh_pressure);
  }

  if (current_dirac_ptid < _zs.size() - 1)
  // contribution from half-segment "ahead of" this point
  {
    wc = wellConstant(_permeability[_qp], _rot_matrix[current_dirac_ptid], _half_seg_len[current_dirac_ptid], _current_elem, _rs[current_dirac_ptid]);
    //Moose::out << " p= " << _u[_qp] << " relperm = " << (*_relperm_val)[_qp] << " density = " << (*_density_val)[_qp] << " mob = " << mob << "\n";
    //Moose::out << " qp= " << _qp << " density = " << _density[_qp][_pvar] << " p = " << std::log(_density[_qp][_pvar]/1000)*2E9 << "\n";
    if ((character < 0.0 && _u[_qp] < bh_pressure) || (character > 0.0 && _u[_qp] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflow += _test[_i][_qp]*std::abs(character)*wc*mob*(_u[_qp] - bh_pressure);
    //outflow += _test[_i][_qp]*std::abs(character)*wc*std::exp(_u[_qp]*10)*(_u[_qp] - bh_pressure)/_viscosity[_qp][_pvar];
  }

  // bodge
  //Real u_from_dens = std::log( (*_density_val)[_qp]/1000)*2E9 ;
  //if (u_from_dens < _u[_qp] - 100 || u_from_dens > _u[_qp] + 100 || (*_relperm_val)[_qp] < 0.99 || _u[_qp] < bh_pressure || wc < 1E-16 || wc > 1E-15)
  //  mooseError("u_from_dens = " << u_from_dens << " u = " << _u[_qp] << " relperm = " << (*_relperm_val)[_qp] << " wc = " << wc << " perm = " << _permeability[_qp] << " rot = " << _rot_matrix[current_dirac_ptid] << " _halflen = " << _half_seg_len[current_dirac_ptid] << " rs = " << _rs[current_dirac_ptid] << "\n");
  //Moose::out << "wc = " << wc << "\n";


  _total_outflow_mass.add(outflow*_dt); // this is not thread safe, but DiracKernel's aren't currently threaded
  return outflow;
}

Real
RichardsBorehole::computeQpJacobian()
{
  Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0) return 0.0;

  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think?!

  Real mob = 1.0/_viscosity[_qp][_pvar];
  mob *= (_using_coupled_vars ? (*_relperm_val)[_qp]*(*_density_val)[_qp] : _rel_perm[_qp][_pvar]*_density[_qp][_pvar]);

  Real mobp = 1.0/_viscosity[_qp][_pvar];
  mobp *= (_using_coupled_vars ? (*_drelperm_val)[_qp]*(*_dseff_val)[_qp]*(*_density_val)[_qp] + (*_relperm_val)[_qp]*(*_ddensity_val)[_qp] : _drel_perm[_qp][_pvar]*_dseff[_qp][_pvar][_pvar]*_density[_qp][_pvar] + _rel_perm[_qp][_pvar]*_ddensity[_qp][_pvar]);

  // when we can query for the current_dirac_ptid replace this complete bodge:
  unsigned int current_dirac_ptid = 0;
  if (_zs.size() > 2)
    current_dirac_ptid = 1;

  Real outflowp(0.0);

  Real wc(0.0);
  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point
  {
    wc = wellConstant(_permeability[_qp], _rot_matrix[current_dirac_ptid - 1], _half_seg_len[current_dirac_ptid - 1], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _u[_qp] < bh_pressure) || (character > 0.0 && _u[_qp] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflowp += _test[_i][_qp]*std::abs(character)*wc*(mob*_phi[_j][_qp] + mobp*_phi[_j][_qp]*(_u[_qp] - bh_pressure));
  }

  if (current_dirac_ptid < _zs.size() - 1)
  // contribution from half-segment "ahead of" this point
  {
    wc = wellConstant(_permeability[_qp], _rot_matrix[current_dirac_ptid], _half_seg_len[current_dirac_ptid], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _u[_qp] < bh_pressure) || (character > 0.0 && _u[_qp] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflowp += _test[_i][_qp]*std::abs(character)*wc*(mob*_phi[_j][_qp] + mobp*_phi[_j][_qp]*(_u[_qp] - bh_pressure));
  }

  return outflowp;
}

Real
RichardsBorehole::computeQpOffDiagJacobian(unsigned int jvar)
{
  Moose::out << "Starting OffDiag computation for borehole\n";
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;
  unsigned int dvar = _richards_name_UO.richards_var_num(jvar);

  Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0) return 0.0;

  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think?!

  Real mobp = _dseff[_qp][_pvar][dvar]/_viscosity[_qp][_pvar];
  mobp *= (_using_coupled_vars ? (*_drelperm_val)[_qp]*(*_density_val)[_qp] : _drel_perm[_qp][_pvar]*_density[_qp][_pvar]);

  // when we can query for the current_dirac_ptid replace this complete bodge:
  unsigned int current_dirac_ptid = 0;
  if (_zs.size() > 2)
    current_dirac_ptid = 1;

  Real outflowp(0.0);

  Real wc(0.0);
  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point
  {
    wc = wellConstant(_permeability[_qp], _rot_matrix[current_dirac_ptid - 1], _half_seg_len[current_dirac_ptid - 1], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _u[_qp] < bh_pressure) || (character > 0.0 && _u[_qp] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflowp += _test[_i][_qp]*std::abs(character)*wc*(mobp*_phi[_j][_qp]*(_u[_qp] - bh_pressure));
  }

  if (current_dirac_ptid < _zs.size() - 1)
  // contribution from half-segment "ahead of" this point
  {
    wc = wellConstant(_permeability[_qp], _rot_matrix[current_dirac_ptid], _half_seg_len[current_dirac_ptid], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _u[_qp] < bh_pressure) || (character > 0.0 && _u[_qp] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflowp += _test[_i][_qp]*std::abs(character)*wc*(mobp*_phi[_j][_qp]*(_u[_qp] - bh_pressure));
  }

  return outflowp;
}
