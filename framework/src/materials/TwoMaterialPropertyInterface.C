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

#include "TwoMaterialPropertyInterface.h"
#include "MaterialData.h"

TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(InputParameters & parameters) :
    MaterialPropertyInterface(parameters),
    _neighbor_material_data(*parameters.get<MaterialData *>("_neighbor_material_data")),
    _neighbor_material_props(_neighbor_material_data.props()),
    _neighbor_material_props_old(_neighbor_material_data.propsOld()),
    _neighbor_material_props_older(_neighbor_material_data.propsOlder())
{
}
