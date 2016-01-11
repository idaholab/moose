/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "Q2PBorehole.h"
#include "RotationMatrix.h"

template<>
InputParameters validParams<Q2PBorehole>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<UserObjectName>("fluid_density", "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredParam<UserObjectName>("fluid_relperm", "A RichardsRelPerm UserObject (eg RichardsRelPermPower) that defines the fluid relative permeability as a function of the saturation Variable.");
  params.addRequiredCoupledVar("other_var", "The other variable in the 2-phase system.  If Variable=porepressure, the other_var=saturation, and vice-versa.");
  params.addRequiredParam<bool>("var_is_porepressure", "This flag is needed to correctly calculate the Jacobian entries.  If set to true, this Sink will extract fluid from the phase with porepressure as its Variable (usually the liquid phase).  If set to false, this Sink will extract fluid from the phase with saturation as its variable (usually the gas phase)");
  params.addRequiredParam<Real>("fluid_viscosity", "The fluid dynamic viscosity");
  params.addRequiredParam<FunctionName>("character", "If zero then borehole does nothing.  If positive the borehole acts as a sink (production well) for porepressure > borehole pressure, and does nothing otherwise.  If negative the borehole acts as a source (injection well) for porepressure < borehole pressure, and does nothing otherwise.  The flow rate to/from the borehole is multiplied by |character|, so usually character = +/- 1, but you can specify other quantities to provide an overall scaling to the flow if you like.");
  params.addRequiredParam<Real>("bottom_pressure", "Pressure at the bottom of the borehole");
  params.addRequiredParam<RealVectorValue>("unit_weight", "(fluid_density*gravitational_acceleration) as a vector pointing downwards.  Note that the borehole pressure at a given z position is bottom_pressure + unit_weight*(p - p_bottom), where p=(x,y,z) and p_bottom=(x,y,z) of the bottom point of the borehole.  If you don't want bottomhole pressure to vary in the borehole just set unit_weight=0.  Typical value is gravity = (0,0,-1E4)");
  params.addRequiredParam<std::string>("point_file", "The file containing the borehole radii and coordinates of the point sinks that approximate the borehole.  Each line in the file must contain a space-separated radius and coordinate.  Ie r x y z.  The last point in the file is defined as the borehole bottom, where the borehole pressure is bottom_pressure.  If your file contains just one point, you must also specify the borehole_length and borehole_direction.  Note that you will get segementation faults if your points do not lie within your mesh!");
  params.addRequiredParam<UserObjectName>("SumQuantityUO", "User Object of type=RichardsSumQuantity in which to place the total outflow from the borehole for each time step.");
  params.addParam<Real>("re_constant", 0.28, "The dimensionless constant used in evaluating the borehole effective radius.  This depends on the meshing scheme.  Peacemann finite-difference calculations give 0.28, while for rectangular finite elements the result is closer to 0.1594.  (See  Eqn(4.13) of Z Chen, Y Zhang, Well flow models for various numerical methods, Int J Num Analysis and Modeling, 3 (2008) 375-388.)");
  params.addParam<Real>("well_constant", -1.0, "Usually this is calculated internally from the element geometry, the local borehole direction and segment length, and the permeability.  However, if this parameter is given as a positive number then this number is used instead of the internal calculation.  This speeds up computation marginally.  re_constant becomes irrelevant");
  params.addRangeCheckedParam<Real>("borehole_length", 0.0, "borehole_length>=0", "Borehole length.  Note this is only used if there is only one point in the point_file.");
  params.addParam<RealVectorValue>("borehole_direction", RealVectorValue(0, 0, 1), "Borehole direction.  Note this is only used if there is only one point in the point_file.");
  params.addClassDescription("Approximates a borehole in the mesh with given bottomhole pressure, and radii using a number of point sinks whose positions are read from a file.  This DiracKernel is for use by Q2P models");
  return params;
}

Q2PBorehole::Q2PBorehole(const InputParameters & parameters) :
    DiracKernel(parameters),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _relperm(getUserObject<RichardsRelPerm>("fluid_relperm")),
    _other_var_nodal(coupledNodalValue("other_var")),
    _other_var_num(coupled("other_var")),
    _var_is_pp(getParam<bool>("var_is_porepressure")),
    _viscosity(getParam<Real>("fluid_viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _character(getFunction("character")),
    _p_bot(getParam<Real>("bottom_pressure")),
    _unit_weight(getParam<RealVectorValue>("unit_weight")),
    _re_constant(getParam<Real>("re_constant")),
    _well_constant(getParam<Real>("well_constant")),
    _borehole_length(getParam<Real>("borehole_length")),
    _borehole_direction(getParam<RealVectorValue>("borehole_direction")),
    _num_nodes(0),
    _pp(0),
    _sat(0),
    _mobility(0),
    _dmobility_dp(0),
    _dmobility_ds(0),
    _total_outflow_mass(const_cast<RichardsSumQuantity &>(getUserObject<RichardsSumQuantity>("SumQuantityUO"))),
    _point_file(getParam<std::string>("point_file"))
{
  // zero the outflow mass
  _total_outflow_mass.zero();

  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _point_file + "' from Q2PBorehole.");

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
  _half_seg_len.resize(std::max(num_pts-1, 1));
  for (unsigned int i = 0 ; i + 1 < _xs.size(); ++i)
  {
    _half_seg_len[i] = 0.5*std::sqrt(std::pow(_xs[i+1] - _xs[i], 2) + std::pow(_ys[i+1] - _ys[i], 2) + std::pow(_zs[i+1] - _zs[i], 2));
    if (_half_seg_len[i] == 0)
      mooseError("Borehole has a zero-segment length at (x,y,z) = " << _xs[i] << " " << _ys[i] << " " << _zs[i] << "\n");
  }
  if (num_pts == 1)
    _half_seg_len[0] = _borehole_length;

  // construct the rotation matrix needed to rotate the permeability
  _rot_matrix.resize(std::max(num_pts-1, 1));
  for (unsigned int i = 0 ; i + 1 < _xs.size(); ++i)
  {
    RealVectorValue v2(_xs[i+1] - _xs[i], _ys[i+1] - _ys[i], _zs[i+1] - _zs[i]);
    _rot_matrix[i] = RotationMatrix::rotVecToZ(v2);
  }
  if (num_pts == 1)
    _rot_matrix[0] = RotationMatrix::rotVecToZ(_borehole_direction);

}

bool
Q2PBorehole::parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec)
// reads a space-separated line of floats from ifs and puts in myvec
{
  std::string line;
  myvec.clear();
  bool gotline(false);
  if (getline(ifs,line))
  {
    gotline = true;

    //Harvest floats separated by whitespace
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
Q2PBorehole::addPoints()
{
  // This function gets called just before the DiracKernel is evaluated
  // so this is a handy place to zero this out.
  _total_outflow_mass.zero();

  // Add point using the unique ID "i", let the DiracKernel take
  // care of the caching.  This should be fast after the first call,
  // as long as the points don't move around.
  for (unsigned int i = 0; i < _zs.size(); i++)
    addPoint(Point(_xs[i], _ys[i], _zs[i]), i);
}

Real
Q2PBorehole::wellConstant(const RealTensorValue & perm, const RealTensorValue & rot, const Real & half_len, const Elem * ele, const Real & rad)
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

  Real r0 = _re_constant*std::sqrt( std::sqrt(eig_val1/eig_val2)*std::pow(ll2, 2) + std::sqrt(eig_val2/eig_val1)*std::pow(ll1, 2)) / ( std::pow(eig_val1/eig_val2, 0.25) + std::pow(eig_val2/eig_val1, 0.25) );

  Real effective_perm = std::sqrt(det2D);

  const Real halfPi = acos(0.0);

  if (r0 <= rad)
    mooseError("The effective element size (about 0.2-times-true-ele-size) for an element containing a Q2PBorehole must be (much) larger than the borehole radius for the Peaceman formulation to be correct.  Your element has effective size " << r0 << " and the borehole radius is " << rad << "\n");

  return 4*halfPi*effective_perm*half_len/std::log(r0/rad);
}

void
Q2PBorehole::prepareNodalValues()
{
  _num_nodes = _other_var_nodal.size();

  // set _pp and _sat variables
  _pp.resize(_num_nodes);
  _sat.resize(_num_nodes);
  if (_var_is_pp)
  {
    for (unsigned int nodenum = 0; nodenum < _num_nodes ; ++nodenum)
    {
      _pp[nodenum] = _var.nodalSln()[nodenum];
      _sat[nodenum] = _other_var_nodal[nodenum];
    }
  }
  else
  {
    for (unsigned int nodenum = 0; nodenum < _num_nodes ; ++nodenum)
    {
      _pp[nodenum] = _other_var_nodal[nodenum];
      _sat[nodenum] = _var.nodalSln()[nodenum];
    }
  }


  Real density;
  Real ddensity_dp;
  Real relperm;
  Real drelperm_ds;
  _mobility.resize(_num_nodes);
  _dmobility_dp.resize(_num_nodes);
  _dmobility_ds.resize(_num_nodes);
  for (unsigned int nodenum = 0; nodenum < _num_nodes ; ++nodenum)
  {
    density = _density.density(_pp[nodenum]);
    ddensity_dp = _density.ddensity(_pp[nodenum]);
    relperm = _relperm.relperm(_sat[nodenum]);
    drelperm_ds = _relperm.drelperm(_sat[nodenum]);
    _mobility[nodenum] = density*relperm/_viscosity;
    _dmobility_dp[nodenum] = ddensity_dp*relperm/_viscosity;
    _dmobility_ds[nodenum] = density*drelperm_ds/_viscosity;
  }
}

void
Q2PBorehole::computeResidual()
{
  prepareNodalValues();
  DiracKernel::computeResidual();
}

Real
Q2PBorehole::computeQpResidual()
{
  Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0) return 0.0;

  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think?!


  // Get the ID we initially assigned to this point
  unsigned current_dirac_ptid = currentPointCachedID();

  // If getting the ID failed, fall back to the old bodge!
  if (current_dirac_ptid == libMesh::invalid_uint)
    current_dirac_ptid = (_zs.size() > 2) ? 1 : 0;

  Real outflow(0.0); // this is the flow rate from porespace out of the system

  Real wc(0.0);
  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point (must have >1 point for current_dirac_ptid>0)
  {
    wc = wellConstant(_permeability[0], _rot_matrix[current_dirac_ptid - 1], _half_seg_len[current_dirac_ptid - 1], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _pp[_i] < bh_pressure) || (character > 0.0 && _pp[_i] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflow += _test[_i][_qp]*std::abs(character)*wc*_mobility[_i]*(_pp[_i] - bh_pressure);
  }

  if (current_dirac_ptid + 1 < _zs.size() || _zs.size() == 1)
  // contribution from half-segment "ahead of" this point, or we only have one point
  {
    wc = wellConstant(_permeability[0], _rot_matrix[current_dirac_ptid], _half_seg_len[current_dirac_ptid], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _pp[_i] < bh_pressure) || (character > 0.0 && _pp[_i] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflow += _test[_i][_qp]*std::abs(character)*wc*_mobility[_i]*(_pp[_i] - bh_pressure);
  }

  _total_outflow_mass.add(outflow*_dt); // this is not thread safe, but DiracKernel's aren't currently threaded
  return outflow;
}

void
Q2PBorehole::computeJacobian()
{
  prepareNodalValues();
  DiracKernel::computeJacobian();
}

Real
Q2PBorehole::computeQpJacobian()
{
  return jac(_var.number());
}

Real
Q2PBorehole::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _other_var_num || jvar == _var.number())
    return jac(jvar);

  return 0.0;
}


Real
Q2PBorehole::jac(unsigned int jvar)
{
  if (_i != _j)
    return 0.0;

  Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0)
    return 0.0;

  Real bh_pressure = _p_bot + _unit_weight*(_q_point[_qp] - _bottom_point); // really want to use _q_point instaed of _current_point, i think?!

  Real phi = 1;

  // Get the ID we initially assigned to this point
  unsigned current_dirac_ptid = currentPointCachedID();

  // If getting the ID failed, fall back to the old bodge!
  if (current_dirac_ptid == libMesh::invalid_uint)
    current_dirac_ptid = (_zs.size() > 2) ? 1 : 0;

  Real outflowp(0.0);

  bool deriv_wrt_pp = (_var_is_pp && (jvar == _var.number())) || (!_var_is_pp && (jvar == _other_var_num));


  Real wc(0.0);
  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point
  {
    wc = wellConstant(_permeability[0], _rot_matrix[current_dirac_ptid - 1], _half_seg_len[current_dirac_ptid - 1], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _pp[_i] < bh_pressure) || (character > 0.0 && _pp[_i] > bh_pressure))
    {
      // injection, so outflow<0 || // production, so outflow>0
      if (deriv_wrt_pp)
        outflowp += std::abs(character)*wc*(_mobility[_i]*phi + _dmobility_dp[_i]*phi*(_pp[_i] - bh_pressure));
      else
        outflowp += std::abs(character)*wc*_dmobility_ds[_i]*phi*(_pp[_i] - bh_pressure);
    }
  }

  if (current_dirac_ptid < _zs.size() - 1 || _zs.size() == 1)
  // contribution from half-segment "ahead of" this point
  {
    wc = wellConstant(_permeability[0], _rot_matrix[current_dirac_ptid], _half_seg_len[current_dirac_ptid], _current_elem, _rs[current_dirac_ptid]);
    if ((character < 0.0 && _pp[_i] < bh_pressure) || (character > 0.0 && _pp[_i] > bh_pressure))
    {
      // injection, so outflow<0 || // production, so outflow>0
      if (deriv_wrt_pp)
        outflowp += std::abs(character)*wc*(_mobility[_i]*phi + _dmobility_dp[_i]*phi*(_pp[_i] - bh_pressure));
      else
        outflowp += std::abs(character)*wc*_dmobility_ds[_i]*phi*(_pp[_i] - bh_pressure);
    }
  }

  return _test[_i][_qp]*outflowp;
}
