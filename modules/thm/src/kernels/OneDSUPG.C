#include "OneDSUPG.h"

registerMooseObject("THMApp", OneDSUPG);

template <>
InputParameters
validParams<OneDSUPG>()
{
  InputParameters params = validParams<OneDStabilizationBase>();

  params.addRequiredParam<unsigned int>("component",
                                        "The component of the matrix/residual to use.");

  return params;
}

OneDSUPG::OneDSUPG(const InputParameters & parameters)
  : OneDStabilizationBase(parameters), _component(getParam<unsigned int>("component"))
{
}

Real
OneDSUPG::computeQpResidual()
{
  return this->supg_residual_contribution(_component);
}

Real
OneDSUPG::computeQpJacobian()
{
  // On-diagonal Jacobian entry for mass is m=0
  Real val = this->supg_jacobian_contribution(/*k=*/_component, /*m=*/_component);

  // // Debugging
  // if (std::abs(val) > 1.e-6)
  //   _console << "OneDSUPG::val(on diag) = " << val << std::endl;

  // Debugging
  // return 0.;

  return val;
}

Real
OneDSUPG::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned m = this->map_moose_var(jvar);

  if (m != libMesh::invalid_uint)
  {
    Real val = this->supg_jacobian_contribution(/*k=*/_component, m);

    // // Debugging
    // if (std::abs(val) > 1.e-6)
    //   _console << "OneDSUPG::val(off diag) = " << val << std::endl;

    // Debugging
    // return 0.;

    return val;
  }
  else
    return 0.;
}
