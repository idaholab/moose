/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RadiationDefectCreation.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<RadiationDefectCreation>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("This material computes the amount the vacancy and interstitial concentrations will increase at each quadrature point due to radiation.");

  params.addRequiredParam<Real>("Vg", "Magnitude of source rate");

  params.addRequiredParam<Real>("x1", "The x coordinate of the lower left-hand corner of the box");
  params.addRequiredParam<Real>("y1", "The y coordinate of the lower left-hand corner of the box");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box");

  params.addRequiredParam<Real>("x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>("y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box");

  params.addParam<bool>("periodic",false, "Is the boundary condition periodic?");

  params.addParam<int>("num_defects",1, "Number of defects created per time step, if 0 then it is random");
  params.addParam<Real>("expected_num",0.0,"For a random number of defects, the expected value per unit area/volume per unit time");

  params.addParam<bool>("coupled_rate", false, "Whether or not the fission rate should be coupled.");
  params.addParam<PostprocessorName>("coupled_rate_name","dummy","The name of the coupled rate");


  params.addRequiredCoupledVar("eta","Order parameter");

  params.addParam<Real>("vac_bias",1.0, "Ratio bias to vacancy creation over interstitial creation; >1 means more vacancies generated");
  params.addParam<Real>("spacing_mult",1.0, "For spacings other than 10 nm");
  params.addParam<Real>("fission_rate_mult",1.0, "Modification to fission rate to make it work better");

  return params;
}

RadiationDefectCreation::RadiationDefectCreation(const InputParameters & parameters) :
    Material(parameters),
    _mesh_dimension(_mesh.dimension()),
    _eta(coupledValue("eta")),
    _interstitial_increase(declareProperty<Real>("interstitial_increase")),
    _vacancy_increase(declareProperty<Real>("vacancy_increase")),
    _Vg(getParam<Real>("Vg")),
    _x1(getParam<Real>("x1")),
    _y1(getParam<Real>("y1")),
    _z1(getParam<Real>("z1")),
    _x2(getParam<Real>("x2")),
    _y2(getParam<Real>("y2")),
    _z2(getParam<Real>("z2")),
    _bottom_left(_x1,_y1,_z1),
    _top_right(_x2,_y2,_z2),
    _range(_top_right - _bottom_left),
    _periodic(getParam<bool>("periodic")),
    _coupled_rate(getParam<bool>("coupled_rate")),
    _input_num_defects(getParam<int>("num_defects")),
    _num_defects(0),
    _input_rate(getParam<Real>("expected_num")),
    _expected_num(getParam<bool>("coupled_rate") ? getPostprocessorValue("coupled_rate_name") : _input_rate),
    _vac_bias(getParam<Real>("vac_bias")),
    _spacing_mult(getParam<Real>("spacing_mult")),
    _fission_rate_mult(getParam<Real>("fission_rate_mult"))
{
}

void
RadiationDefectCreation::timestepSetup()
{
  MooseRandom::seed(_t_step);

  Real scaled_expected_num = _expected_num/_fission_rate_mult;
  Real scaled_Vg = _Vg*_fission_rate_mult;

  if (_coupled_rate)
    scaled_expected_num *= (1e-8)*(1e-8)*(1e-8); // convert to (10 nm)^3

  _mag = scaled_Vg/_dt;

  // If the number of neutrons is not specified, randomly generate it
  if (_input_num_defects == 0 && scaled_expected_num > 0)
  {
    Real meanvalue = _range(0)*_range(1)*scaled_expected_num*_dt;
    if (_mesh_dimension > 2)
      meanvalue *= _range(2);

    Real std_dev = 1.0;

    Real rnd1 = MooseRandom::rand();
    Real rnd2 = MooseRandom::rand();

    Real std_normal = std::sqrt(-2.0 * std::log(rnd1)) * std::cos(2.0 * libMesh::pi * rnd2);

    // The 0.5 is for rounding
    _num_defects = (std_normal*std_dev + meanvalue) + 0.5;

    if (_t_step==1 && _num_defects<1)
      _num_defects = 1;

    if (_num_defects<0)
      _num_defects = 0;

    if (_num_defects == 0)
      _mag = 0.0;
  }
  else if (_input_num_defects > 0)
    _num_defects = _input_num_defects;

  // Resize vector of neutron impact positions
  _NImpPos.resize(_num_defects);

  // Randomly determine the neutron impact positions
  for (int i=0; i<_num_defects; i++)
  {
    _NImpPos[i](0)=_x1 + _range(0) * MooseRandom::rand();
    _NImpPos[i](1)=_y1 + _range(1) * MooseRandom::rand();
    if (_mesh_dimension>2)
      _NImpPos[i](2)=_z1 + _range(2) * MooseRandom::rand();
    else
      _NImpPos[i](2) = 0.0;
  }
}

void
RadiationDefectCreation::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    Real QPVacIncr = 0.0;
    Real QPIntIncr = 0.0;

    if (_t_step > 0 && _NImpPos.size() > 0)
    {
      Point p_difference;

      for (int ht=0; ht<_num_defects; ht++)
      {
        // Determine the distance from the current QP to the impact position
        p_difference = _q_point[_qp] - _NImpPos[ht];

        if (_periodic)
        {
          for (unsigned int i=0; i<LIBMESH_DIM; i++)
            if (std::abs(p_difference(i)) > _range(i)/2)
              p_difference(i) = _range(i) - std::abs(_q_point[_qp](i) - _NImpPos[ht](i));
        }

        Real rad = _spacing_mult*p_difference.size();

        // Set the vacancy and interstitial increases

        Real gamma = 1.0/libMesh::pi; // Determines shape of distribution functions;
        Real cauchy_vac = gamma/libMesh::pi*(1.0/(rad*rad + gamma*gamma)); // distribution function
        Real rad_int = rad-1;
        Real cauchy_int = gamma/libMesh::pi * (1.0/(rad_int * rad_int + gamma*gamma)); // for interstitials
        const Real vi_ratio = 0.22722130528; // balances volume so new interstitials=new vacancies

        QPVacIncr += _vac_bias*_mag * cauchy_vac *(1.0 - _eta[_qp]*_eta[_qp]*_eta[_qp]);
        QPIntIncr += vi_ratio*_mag * cauchy_int *(1.0 - _eta[_qp]*_eta[_qp]*_eta[_qp]);
      }

      if (_eta[_qp] > .9999)
      {
        QPVacIncr = 0.0;
        QPIntIncr = 0.0;
      }
    }
    _vacancy_increase[_qp] = QPVacIncr;
    _interstitial_increase[_qp] = QPIntIncr;
  }
}
