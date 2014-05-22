#ifndef GEOTHERMALMATERIALACTION_H
#define GEOTHERMALMATERIALACTION_H

#include "Action.h"

class GeothermalMaterialAction;

template<>
InputParameters validParams<GeothermalMaterialAction>();

class GeothermalMaterialAction : public Action
{
public:
  GeothermalMaterialAction(const std::string & name, InputParameters params);

  virtual void act();

  virtual void addSolidMechanicsMaterial(InputParameters shared_params);
  virtual void addHeatTransportMaterial(InputParameters shared_params);
  virtual void addFluidFlowMaterial(InputParameters shared_params);
  virtual void addChemicalReactionsMaterial(InputParameters shared_params);

protected:
  bool _has_heat_tran;
  bool _has_fluid_flow;
  bool _has_solid_mech;
  bool _has_chem_react;
};

#endif /* GEOTHERMALMATERIALACTION_H */
