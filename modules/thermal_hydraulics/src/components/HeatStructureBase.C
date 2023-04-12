//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureBase.h"
#include "SolidMaterialProperties.h"
#include "ConstantFunction.h"

InputParameters
HeatStructureBase::validParams()
{
  InputParameters params = Component2D::validParams();
  params += HeatStructureInterface::validParams();
  return params;
}

HeatStructureBase::HeatStructureBase(const InputParameters & params)
  : Component2D(params),
    HeatStructureInterface(this),
    _connected_to_flow_channel(false),
    _number_of_hs(_n_regions)
{
}

void
HeatStructureBase::init()
{
  Component2D::init();
  HeatStructureInterface::init();
}

void
HeatStructureBase::check() const
{
  Component2D::check();
  HeatStructureInterface::check();
}

const unsigned int &
HeatStructureBase::getIndexFromName(const std::string & name) const
{
  return _name_index.at(name);
}

bool
HeatStructureBase::usingSecondOrderMesh() const
{
  return HeatConductionModel::feType().order == SECOND;
}

void
HeatStructureBase::addVariables()
{
  HeatStructureInterface::addVariables();
}

void
HeatStructureBase::addMooseObjects()
{
  HeatStructureInterface::addMooseObjects();

  if (isParamValid("materials"))
  {
    _hc_model->addMaterials();

    for (unsigned int i = 0; i < _n_regions; i++)
    {
      const SolidMaterialProperties & smp =
          getTHMProblem().getUserObject<SolidMaterialProperties>(_material_names[i]);

      Component * comp = (_parent != nullptr) ? _parent : this;
      // if the values were given as constant, allow them to be controlled
      const ConstantFunction * k_fn = dynamic_cast<const ConstantFunction *>(&smp.getKFunction());
      if (k_fn != nullptr)
        comp->connectObject(k_fn->parameters(), k_fn->name(), "k", "value");

      const ConstantFunction * cp_fn = dynamic_cast<const ConstantFunction *>(&smp.getCpFunction());
      if (cp_fn != nullptr)
        comp->connectObject(cp_fn->parameters(), cp_fn->name(), "cp", "value");

      const ConstantFunction * rho_fn =
          dynamic_cast<const ConstantFunction *>(&smp.getRhoFunction());
      if (rho_fn != nullptr)
        comp->connectObject(rho_fn->parameters(), rho_fn->name(), "rho", "value");
    }
  }
}
