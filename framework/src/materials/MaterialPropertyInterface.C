#include "MaterialPropertyInterface.h"

#include "MaterialData.h"
#include "MooseSystem.h"

MaterialPropertyInterface::MaterialPropertyInterface(MaterialData & material_data):
  _material_data(material_data)
{}

