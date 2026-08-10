#include "KLNormalAux.h"
#include "KLE_uo.h"

registerMooseObject("MooseApp", KLNormalAux);

InputParameters
KLNormalAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Generates a sample of centered Normal random field with mean and covariance "
      "specified from KLExpansionUserObject.");
  params.addRequiredParam<UserObjectName>("kl_user_object",
                                          "Name of the KLExpansionUserObject to sample");
  params.addParam<Real>("mean", 0.0, "Constant value added to the raw KL Normal before output");
  return params;
}

KLNormalAux::KLNormalAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _kl_uo(getUserObject<KLExpansionUserObject>("kl_user_object")),
    _mean(getParam<Real>("mean"))
{
}

Real
KLNormalAux::computeValue()
{
  const Point & p = isNodal() ? *_current_node : _q_point[_qp];
  return _mean + _kl_uo.getValue(p);
}
