#include "ConvectionDiffusionSUPG.h"

template<>
InputParameters validParams<ConvectionDiffusionSUPG>()
{
  InputParameters params;
  params.addRequiredParam<Real>("coef", "The diffusion coefficient.");
  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addParam<Real>("y", 0.0, "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

ConvectionDiffusionSUPG::ConvectionDiffusionSUPG(std::string name,
                       InputParameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as)
  :Stabilizer(name, parameters, var_name, coupled_to, coupled_as),
   _coef(_parameters.get<Real>("coef")),
   _x(_parameters.get<Real>("x")),
   _y(_parameters.get<Real>("y")),
   _z(_parameters.get<Real>("z"))
{
  _velocity(0)=_x;
  _velocity(1)=_y;
  _velocity(2)=_z;

  _vel_mag = _velocity.size();
}

void
ConvectionDiffusionSUPG::computeTestFunctions()
{
  // Copy the pure shape functions into the test functions
  _test = _phi;

  Real h = _current_elem->hmin();
  
  Real pec = (_vel_mag*h) / (2.0*_coef);

  Real tau = (h/(2.0*_vel_mag))*(coth(pec - (1.0/pec)));

  unsigned int num_q_points = _qrule->n_points();
  unsigned int num_shape = _phi.size();
  
  for(unsigned int i=0; i<num_shape; i++)
    for(unsigned int qp=0; qp<num_q_points; qp++)
      _test[i][qp] += tau*(_velocity*_dphi[i][qp]);
}

Real
ConvectionDiffusionSUPG::coth(Real x)
{
  Real exp_cache = exp(2.0*x);
  return (exp_cache - 1) / (exp_cache + 1);
}
