/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PrintMaterials.h"

#include "ActionWarehouse.h"
#include "AddMaterialAction.h"
#include "MooseApp.h"

#include <fstream>

template<>
InputParameters validParams<PrintMaterials>()
{
  InputParameters params = validParams<Action>();
  params.addParam<FileName>("csv_file", "matinfo.csv", "CSV file for printing the material information");
  return params;
}

PrintMaterials::PrintMaterials(const InputParameters & params) :
    Action(params)
{
}

void
PrintMaterials::act()
{
  // only act on master processor
  if (_app.processor_id() == 0)
  {
    std::set<const AddMaterialAction *> actions = _awh.getActions<AddMaterialAction>();

    std::map<std::string, unsigned int> map_type;
    map_type["GenericConstantMaterial"] = 100;
    map_type["CoupledMaterial"] = 888;

    std::ofstream os(getParam<FileName>("csv_file").c_str());
    os << "name_id,type_id" << std::endl;
    unsigned int i=0;
    for (std::set<const AddMaterialAction *>::iterator it = actions.begin();
         it != actions.end(); ++it)
      os << i++ << "," << map_type[(*it)->getMooseObjectType()] << std::endl;
    os.close();

    if (actions.size()>0)
    {
      const AddMaterialAction & action = _awh.getAction<AddMaterialAction>((*actions.begin())->name());
      if (*actions.begin() != (&action))
        mooseError("something is really wrong");
    }
  }
}
