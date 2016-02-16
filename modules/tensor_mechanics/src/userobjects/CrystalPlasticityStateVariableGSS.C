/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrystalPlasticityStateVariableGSS.h"

template<>
InputParameters validParams<CrystalPlasticityStateVariableGSS>()
{
  InputParameters params = validParams<CrystalPlasticityStateVariable>();
  params.addParam<std::vector<Real> >("gprops", "Initial values of slip system resistances");
  params.addClassDescription("Phenomenological constitutive model state class.  Override the virtual functions in your class");
  return params;
}

CrystalPlasticityStateVariableGSS::CrystalPlasticityStateVariableGSS(const InputParameters & parameters) :
    CrystalPlasticityStateVariable(parameters),
    _gprops(getParam<std::vector<Real> >("gprops"))
{
}

void
CrystalPlasticityStateVariableGSS::initSlipSysProps(std::vector<Real> & val) const
{
  switch (_intvar_read_type)
  {
    case 0:
      assignSlipSysRes(val);
      break;
    case 1:
      readFileInitSlipSysRes(val);
      break;
    default:
      getInitSlipSysRes(val);
  }
}

void
CrystalPlasticityStateVariableGSS::assignSlipSysRes(std::vector<Real> & val) const
{
}

void
CrystalPlasticityStateVariableGSS::getInitSlipSysRes(std::vector<Real> & val) const
{
  if (_gprops.size() <= 0)
    mooseError("CrystalPlasticityStateVariableGSS: Error in reading slip system resistance properties: Specify input in .i file or in slip_sys_res_prop_file or in slip_sys_file");

  // Number of data per group e.g. start_slip_sys, end_slip_sys, value
  unsigned int num_data_grp = 3;

  for (unsigned int i = 0; i < _gprops.size() / num_data_grp; ++i)
  {
    Real vs,ve;
    unsigned int is, ie;

    vs = _gprops[i * num_data_grp];
    ve = _gprops[i * num_data_grp + 1];

    if (vs <= 0 || ve <= 0)
      mooseError( "CrystalPlasticityStateVariableGSS: Indices in gss property read must be positive integers: is = " << vs << " ie = " << ve );

    if (vs != std::floor(vs) || ve != std::floor(ve))
      mooseError("CrystalPlasticityStateVariableGSS: Error in reading slip system resistances: Values specifying start and end number of slip system groups should be integer");

    is = static_cast<unsigned int>(vs);
    ie = static_cast<unsigned int>(ve);

    if (is > ie)
      mooseError("CrystalPlasticityStateVariableGSS: Start index is = " << is << " should be greater than end index ie = " << ie << " in slip system resistance property read");

    for (unsigned int j = is; j <= ie; ++j)
      val[j-1] = _gprops[i * num_data_grp + 2];
  }

  for (unsigned int i = 0; i < _variable_size; ++i)
    if (val[i] <= 0.0)
      mooseError("CrystalPlasticityStateVariableGSS: Value of resistance for slip system " << i + 1 << " non positive");
}
