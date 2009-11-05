#include "SolidMech.h"

template<>
Parameters valid_params<SolidMech>()
{
  Parameters params;
  return params;
}

SolidMech::SolidMech(std::string name,
            Parameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as)
  {}

void
SolidMech::subdomainSetup()
  {
    _E_prop = &_material->getRealProperty("youngs_modulus");
    _nu_prop = &_material->getRealProperty("poissons_ratio");
  }

void
SolidMech::recomputeConstants()
  {
    _E = (*_E_prop)[_qp];
    _nu = (*_nu_prop)[_qp];

    _c1 = _E*(1.-_nu)/(1.+_nu)/(1.-2.*_nu);
    _c2 = _nu/(1.-_nu);
    _c3 = .5*(1.-2.*_nu)/(1.-_nu);

    _B11=TensorValue<Number>(1., 0., 0.,
			     0., _c3, 0.,
			     0., 0., _c3);

    _B12=TensorValue<Number>(0., _c2, 0.,
			     _c3, 0., 0.,
			     0., 0., 0.);

    _B13=TensorValue<Number>(0., 0., _c2,
			     0., 0., 0.,
			     _c3, 0., 0.);

    
    _B21=TensorValue<Number>(0., _c3, 0.,
                             _c2, 0., 0.,
			     0., 0., 0.);

    _B22=TensorValue<Number>(_c3, 0., 0.,
			     0., 1., 0.,
			     0., 0., _c3);

    _B23=TensorValue<Number>(0., 0., 0.,
			     0., 0., _c2,
			     0., _c3, 0.);


    
    _B31=TensorValue<Number>(0., 0., _c3,
			     0., 0., 0.,
			     _c2, 0., 0.);

    _B32=TensorValue<Number>(0., 0., 0.,
			     0., 0., _c3,
			     0., _c2, 0.);

    _B33=TensorValue<Number>(_c3, 0., 0.,
			     0., _c3, 0.,
			     0., 0., 1.);
  }
