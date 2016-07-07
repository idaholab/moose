/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


// Creates multiple superellipsoids that are positioned randomly throughout the domain
// each semiaxis can be varied by a uniform or normal distribution


#include "MultiSmoothSuperellipsoidIC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<MultiSmoothSuperellipsoidIC>()
{
  InputParameters params = validParams<SmoothSuperellipsoidBaseIC>();
  params.addClassDescription("Random distribution of smooth ellipse with given minimum spacing");
  params.addRequiredParam<std::vector<unsigned int> >("numbub", "Vector of the number of bubbles to place");
  params.addRequiredParam<std::vector<Real> >("bubspac", "Vector of the minimum spacing of bubbles of one type, measured from center to center");
  params.addParam<unsigned int>("max_num_tries", 1000, "The number of tries");
  params.addRequiredParam<std::vector<Real> >("semiaxis_a", "Vector of mean semiaxis values in the x direction for the ellipse");
  params.addRequiredParam<std::vector<Real> >("semiaxis_b", "Vector of mean semiaxis values in the y direction for the ellipse");
  params.addRequiredParam<std::vector<Real> >("semiaxis_c", "Vector of mean semiaxis values in the z direction for the ellipse, must be set to 1 if 2D.");
  params.addParam<std::vector<Real> >("exponent", std::vector<Real>(),"Vector of exponents for each superellipsoid, n=2 is a normal ellipse");
  params.addParam<std::vector<Real> >("semiaxis_a_variation", std::vector<Real>(), "Vector of plus or minus fractions of random variation in the bubble semiaxis in the x direction for uniform, standard deviation for normal");
  params.addParam<std::vector<Real> >("semiaxis_b_variation", std::vector<Real>(), "Vector of plus or minus fractions of random variation in the bubble semiaxis in the y direction for uniform, standard deviation for normal");
  params.addParam<std::vector<Real> >("semiaxis_c_variation", std::vector<Real>(), "Vector of plus or minus fractions of random variation in the bubble semiaxis in the z direction for uniform, standard deviation for normal. Must be set to 0 if 2D.");
  params.addParam<bool>("check_extremes", false, "Check all Superellipsoid extremes (center +- each semiaxis) for overlap, must have prevent_overlap set to True.");
  params.addParam<bool>("prevent_overlap", false, "Check all Superellipsoid centers for overlap with other Superellipsoids.");
  params.addParam<bool>("vary_axes_independently", true, "If true the length of each semiaxis is randomly chosen within the provided parameters, if false then one random number is generated and applied to all semiaxes.");
  MooseEnum rand_options("uniform normal none","none");
  params.addParam<MooseEnum>("semiaxis_variation_type", rand_options, "Type of distribution that random superellipsoid semiaxes will follow");
  return params;
}

MultiSmoothSuperellipsoidIC::MultiSmoothSuperellipsoidIC(const InputParameters & parameters) :
    SmoothSuperellipsoidBaseIC(parameters),
    _max_num_tries(getParam<unsigned int>("max_num_tries")),
    _semiaxis_variation_type(getParam<MooseEnum>("semiaxis_variation_type")),
    _prevent_overlap(getParam<bool>("prevent_overlap")),
    _check_extremes(getParam<bool>("check_extremes")),
    _vary_axes_independently(getParam<bool>("vary_axes_independently")),
    _numbub(parameters.get<std::vector<unsigned int> >("numbub")),
    _bubspac(parameters.get<std::vector<Real> >("bubspac")),
    _exponent(parameters.get<std::vector<Real> >("exponent")),
    _semiaxis_a(parameters.get<std::vector<Real> >("semiaxis_a")),
    _semiaxis_b(parameters.get<std::vector<Real> >("semiaxis_b")),
    _semiaxis_c(parameters.get<std::vector<Real> >("semiaxis_c")),
    _semiaxis_a_variation(parameters.get<std::vector<Real> >("semiaxis_a_variation")),
    _semiaxis_b_variation(parameters.get<std::vector<Real> >("semiaxis_b_variation")),
    _semiaxis_c_variation(parameters.get<std::vector<Real> >("semiaxis_c_variation"))
{
}

void
MultiSmoothSuperellipsoidIC::initialSetup()
{
  Real nv = _numbub.size();

  if (nv != _bubspac.size() || nv != _exponent.size() || nv != _semiaxis_a.size() || nv != _semiaxis_b.size()|| nv != _semiaxis_c.size())
    mooseError("Vectors for numbub, bubspac, exponent, semiaxis_a, semiaxis_b, and semiaxis_c must be the same size.");

  if (_semiaxis_variation_type != 2)
  {
    if (nv != _semiaxis_a_variation.size() || nv != _semiaxis_b_variation.size() || nv != _semiaxis_c_variation.size())
      mooseError("Vectors for numbub, semiaxis_a_variation, semiaxis_b_variation, and semiaxis_c_variation must be the same size.");
  }

  for (unsigned int k = 0; k < nv; k++)
  {  //Set up domain bounds with mesh tools
    _gk = k;
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    {
      _bottom_left(i) = _mesh.getMinInDimension(i);
      _top_right(i) = _mesh.getMaxInDimension(i);
    }
    _range = _top_right - _bottom_left;

    if (_semiaxis_a_variation[_gk] > 0.0 && _semiaxis_variation_type == 2)
      mooseError("If Semiaxis_a_variation > 0.0, you must pass in a Semiaxis_variation_type in MultiSmoothSuperellipsoidIC");
    if (_semiaxis_b_variation[_gk] > 0.0 && _semiaxis_variation_type == 2)
      mooseError("If Semiaxis_b_variation > 0.0, you must pass in a Semiaxis_variation_type in MultiSmoothSuperellipsoidIC");
    if (_semiaxis_c_variation[_gk] > 0.0 && _semiaxis_variation_type == 2)
      mooseError("If Semiaxis_c_variation > 0.0, you must pass in a Semiaxis_variation_type in MultiSmoothSuperellipsoidIC");

    SmoothSuperellipsoidBaseIC::initialSetup();
  }
}

void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidSemiaxes()
{
  Real randnum;
  unsigned int start = _as.size();
  _as.resize(start + _numbub[_gk]);
  _bs.resize(start + _numbub[_gk]);
  _cs.resize(start + _numbub[_gk]);

  for (unsigned int i = start ; i < _as.size(); i++)
  {
    switch (_semiaxis_variation_type)
    {
      case 0: //Uniform distrubtion
        randnum = _random.rand(_tid);
        _as[i] = _semiaxis_a[_gk] * (1.0 + (1.0 - 2.0 * randnum) * _semiaxis_a_variation[_gk]);
        _bs[i] = _semiaxis_b[_gk] * (1.0 + (1.0 - 2.0 * (_vary_axes_independently ? _random.rand(_tid) : randnum)) * _semiaxis_b_variation[_gk]);
        _cs[i] = _semiaxis_c[_gk] * (1.0 + (1.0 - 2.0 * (_vary_axes_independently ? _random.rand(_tid) : randnum)) * _semiaxis_c_variation[_gk]);
        break;

      case 1: //Normal distribution
        randnum = _random.randNormal(_tid, 0, 1);
        _as[i] = _semiaxis_a[_gk] + (randnum * _semiaxis_a_variation[_gk]);
        _bs[i] = _vary_axes_independently ? _random.randNormal(_tid, _semiaxis_b[_gk],_semiaxis_b_variation[_gk]) : _semiaxis_b[_gk] + (randnum * _semiaxis_b_variation[_gk]);
        _cs[i] = _vary_axes_independently ? _random.randNormal(_tid, _semiaxis_c[_gk],_semiaxis_c_variation[_gk]) : _semiaxis_c[_gk] + (randnum * _semiaxis_c_variation[_gk]);
        break;

      case 2: //No variation
        _as[i] = _semiaxis_a[_gk];
        _bs[i] = _semiaxis_b[_gk];
        _cs[i] = _semiaxis_c[_gk];
    }
    _as[i] = _as[i] < 0.0 ? 0.0 : _as[i];
    _bs[i] = _bs[i] < 0.0 ? 0.0 : _bs[i];
    _cs[i] = _cs[i] < 0.0 ? 0.0 : _cs[i];
  }
}

void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidCenters()
{
  unsigned int start = _centers.size();
  _centers.resize(start + _numbub[_gk]);

  for (unsigned int i = start; i < _centers.size(); i++)
  {
    unsigned int num_tries = 0;
    Real rr = 0.0;
    Point newcenter = 0.0;

    //Vary Superellipsoid center positions
    while (rr <= _bubspac[_gk] && num_tries < _max_num_tries)
    {
      num_tries++;
      rr = _bubspac[_gk] + 1;
      Real ran1 = _random.rand(_tid);
      Real ran2 = _random.rand(_tid);
      Real ran3 = _random.rand(_tid);

      newcenter(0) = _bottom_left(0) + ran1 * _range(0);
      newcenter(1) = _bottom_left(1) + ran2 * _range(1);
      newcenter(2) = _bottom_left(2) + ran3 * _range(2);

      for (unsigned int j = start; j < i; j++)
      {
        Real tmp_rr = _mesh.minPeriodicDistance(_var.number(), _centers[j], newcenter);

        if (tmp_rr < _bubspac[_gk])
          rr = tmp_rr;
      }

      //Check each new center candidate against all accepted ellisoids
      if (_prevent_overlap && rr > _bubspac[_gk])
      {
        Real status = overlapCheck(newcenter, _as[i], _bs[i], _cs[i], _ns[i]);
        //If candidate center fails overlapCheck ensure that the loop continues
        if (status)
          rr = _bubspac[_gk];
      }
    }

    if (num_tries == _max_num_tries)
      mooseError("Too many tries in MultiSuperellipsoidEllipseIC");

    _centers[i] = newcenter;
  }
}

void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidExponents()
{
  unsigned int start = _ns.size();
  _ns.resize(start + _numbub[_gk]);

  for (unsigned int i = start; i < _ns.size(); ++i)
    _ns[i] = _exponent[_gk];
}

bool
MultiSmoothSuperellipsoidIC::overlapCheck(const Point & newcenter, Real nc_as, Real nc_bs, Real nc_cs, Real nc_ns)
{
  Point l_p = newcenter;
  const Real a = nc_as;
  const Real b = nc_bs;
  const Real c = nc_cs;
  const Real n = nc_ns;

  //Check for overlap between candidate center and accepted centers
  //Compute the distance between the current point and the center
  for (unsigned int el = 0; el < _centers.size(); ++el)
  {
    Real dist = _mesh.minPeriodicDistance(_var.number(), _centers[el], l_p);
    //When dist is 0 we are exactly at the center of the superellipsoid so return _invalue
    //Handle this case independently because we cannot calculate polar angles at this point
    if (dist == 0.0)
      return true;

    //Compute the distance r from the center of the superellipsoid to its outside edge
    //along the vector from the center to the current point
    //This uses the equation for a superellipse in polar coordinates and substitutes
    //distances for sin, cos functions
    Point dist_vec = _mesh.minPeriodicVector(_var.number(), _centers[el], l_p);

    //First calculate rmn = r^(-n), replacing sin, cos functions with distances
    Real rmn = (std::pow(std::abs(dist_vec(0) / dist / a), n)
              + std::pow(std::abs(dist_vec(1) / dist / b), n)
              + std::pow(std::abs(dist_vec(2) / dist / c), n) );
    //Then calculate r1 from rmn
    Real r1 = std::pow(rmn, (-1.0/n));

    dist_vec = _mesh.minPeriodicVector(_var.number(), l_p, _centers[el]);

    //First calculate rmn = r^(-n), replacing sin, cos functions with distances
    rmn = (std::pow(std::abs(dist_vec(0) / dist / _as[el]), _ns[el])
        + std::pow(std::abs(dist_vec(1) / dist / _bs[el]), _ns[el])
        + std::pow(std::abs(dist_vec(2) / dist / _cs[el]), _ns[el]) );
    //Then calculate r2 from rmn
    Real r2 = std::pow(rmn, (-1.0/_ns[el]));

    if (dist < r1 + r2)
      return true;
  }

  //Check for overlap between extremes of new ellipsoid candidate and the center
  //of accepted ellisoids if _check_extremes enabled
  if (_check_extremes)
  {
    Point tmp_p;
    for (unsigned int pc = 0; pc < 6; pc++)
    {
      tmp_p = l_p;
      //Find extremes along semiaxis of candidate ellipsoids
      if (pc == 0)
        tmp_p(0) -= a;
      else if (pc == 1)
        tmp_p(0) += a;
      else if (pc == 2)
        tmp_p(1) -= b;
      else if (pc == 3 )
        tmp_p(1) += b;
      else if (pc == 4 )
        tmp_p(2) -= c;
      else
        tmp_p(2) += c;

      for (unsigned int el = 0; el < _centers.size(); ++el)
      {
        Real dist = _mesh.minPeriodicDistance(_var.number(), _centers[el], tmp_p);
        //When dist is 0 we are exactly at the center of the superellipsoid so return _invalue
        //Handle this case independently because we cannot calculate polar angles at this point
        if (dist == 0.0)
          return true;

        //Compute the distance r from the center of the superellipsoid to its outside edge
        //along the vector from the center to the current point
        //This uses the equation for a superellipse in polar coordinates and substitutes
        //distances for sin, cos functions
        Point dist_vec = _mesh.minPeriodicVector(_var.number(), _centers[el], tmp_p);

        //First calculate rmn = r^(-n), replacing sin, cos functions with distances
        Real rmn = (std::pow(std::abs(dist_vec(0) / dist / _as[el]), _ns[el])
                  + std::pow(std::abs(dist_vec(1) / dist / _bs[el]), _ns[el])
                  + std::pow(std::abs(dist_vec(2) / dist / _cs[el]), _ns[el]) );
        //Then calculate r2 from rmn
        Real r2 = std::pow(rmn, (-1.0/_ns[el]));

        if (dist < r2)
          return true;
      }
    }

    //Check for overlap between center of new ellipsoid candidate and the extremes of accepted ellisoids
    for (unsigned int el = 0; el < _centers.size(); ++el)
    {
      for (unsigned int pc = 0; pc < 6; pc++)
      {
        tmp_p = _centers[el];
        //Find extremes along semiaxis of accepted ellipsoids
        if (pc == 0)
          tmp_p(0) -= _as[el];
        else if (pc == 1)
          tmp_p(0) += _as[el];
        else if (pc == 2)
          tmp_p(1) -= _bs[el];
        else if (pc == 3 )
          tmp_p(1) += _bs[el];
        else if (pc == 4 )
          tmp_p(2) -= _cs[el];
        else
          tmp_p(2) += _cs[el];

        Real dist = _mesh.minPeriodicDistance(_var.number(), tmp_p, l_p);
        //When dist is 0 we are exactly at the center of the superellipsoid so return _invalue
        //Handle this case independently because we cannot calculate polar angles at this point
        if (dist == 0.0)
          return true;

        //Compute the distance r from the center of the superellipsoid to its outside edge
        //along the vector from the center to the current point
        //This uses the equation for a superellipse in polar coordinates and substitutes
        //distances for sin, cos functions
        Point dist_vec = _mesh.minPeriodicVector(_var.number(), l_p, tmp_p);

        //First calculate rmn = r^(-n), replacing sin, cos functions with distances
        Real rmn = (std::pow(std::abs(dist_vec(0) / dist / a), n)
                  + std::pow(std::abs(dist_vec(1) / dist / b), n)
                  + std::pow(std::abs(dist_vec(2) / dist / c), n) );
        //Then calculate r2 from rmn
        Real r2 = std::pow(rmn, (-1.0/n));

        if (dist < r2)
          return true;
      }
    }
  }
  return false;
}
