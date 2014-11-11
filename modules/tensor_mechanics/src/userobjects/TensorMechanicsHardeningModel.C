//  Hardening model base class.
//
#include "TensorMechanicsHardeningModel.h"

template<>
InputParameters validParams<TensorMechanicsHardeningModel>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Hardening Model base class.  Override the virtual functions in your class");
  return params;
}

TensorMechanicsHardeningModel::TensorMechanicsHardeningModel(const std::string & name, InputParameters parameters) :
  GeneralUserObject(name, parameters)
{
}

void
TensorMechanicsHardeningModel::initialize()
{}

void
TensorMechanicsHardeningModel::execute()
{}

void TensorMechanicsHardeningModel::finalize()
{}

Real
TensorMechanicsHardeningModel::value(const Real & /*intnl*/) const
{
  return 1.0;
}

Real
TensorMechanicsHardeningModel::derivative(const Real & /*intnl*/) const
{
  return 0.0;
}
