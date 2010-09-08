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
#include "MooseSystem.h"

TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(MaterialData & material_data, MaterialData & neighbor_material_data) :
  MaterialPropertyInterface(material_data),
  _neighbor_material_data(neighbor_material_data)
{
}
