#include "SolidMechanicsMaterialRZ.h"

#include "MooseSystem.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<SolidMechanicsMaterialRZ>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("disp_r", "The r displacement");
  params.addRequiredCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  return params;
}

SolidMechanicsMaterialRZ::SolidMechanicsMaterialRZ(const std::string & name,
                                                   InputParameters parameters)
  :Material(name, parameters),
   _initialized(false),
   _disp_r(coupledValue("disp_r")),
   _disp_z(coupledValue("disp_z")),
   _grad_disp_r(coupledGradient("disp_r")),
   _grad_disp_z(coupledGradient("disp_z")),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _volumetric_models(0),
   _stress(declareProperty<RealTensorValue>("stress")),
   _elasticity_tensor(declareProperty<ColumnMajorMatrix>("elasticity_tensor")),
   _Jacobian_mult(declareProperty<ColumnMajorMatrix>("Jacobian_mult")),
   _elastic_strain(declareProperty<ColumnMajorMatrix>("elastic_strain"))
{}

void
SolidMechanicsMaterialRZ::subdomainSetup()
{

  if (!_initialized)
  {
    _initialized = true;

    // Load in the volumetric models
    const std::vector<Material*> mats = _moose_system.getMaterials( _tid, _block_id );
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
