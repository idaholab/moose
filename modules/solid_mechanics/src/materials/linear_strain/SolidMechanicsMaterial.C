#include "SolidMechanicsMaterial.h"
#include "Problem.h"

#include "VolumetricModel.h"

template<>
InputParameters validParams<SolidMechanicsMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  return params;
}

SolidMechanicsMaterial::SolidMechanicsMaterial(const std::string & name, InputParameters parameters)
  :Material(name, parameters),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y")),
   _grad_disp_z(_dim == 3 ? coupledGradient("disp_z") : _grad_zero),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _volumetric_models(0),
   _stress(declareProperty<RealTensorValue>("stress")),
   _elasticity_tensor(declareProperty<ColumnMajorMatrix>("elasticity_tensor")),
   _Jacobian_mult(declareProperty<ColumnMajorMatrix>("Jacobian_mult")),
   _elastic_strain(declareProperty<ColumnMajorMatrix>("elastic_strain")),
   _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
   _density(declareProperty<Real>("density")),
   _specific_heat(declareProperty<Real>("specific_heat"))
{}

void
SolidMechanicsMaterial::subdomainSetup()
{

  if (!_initialized)
  {
    _initialized = true;

    // Load in the volumetric models
    const std::vector<Material*> & mats = _problem.getMaterials( _block_id, _tid );
    for (unsigned int i(0); i < mats.size(); ++i)
    {
      VolumetricModel * vm(dynamic_cast<VolumetricModel*>(mats[i]));
      if (vm)
      {
        _volumetric_models.push_back( vm );
      }
    }
  }
}
