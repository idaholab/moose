#include "TwoMaterialPropertyInterface.h"

#include "MaterialData.h"
#include "MooseSystem.h"

TwoMaterialPropertyInterface::TwoMaterialPropertyInterface(MaterialData & material_data, MaterialData & neighbor_material_data) :
  MaterialPropertyInterface(material_data),
  _neighbor_material_data(neighbor_material_data)
{
}
