//Moose includes
#include "AuxData.h"
#include "ElementData.h"

//libmesh includes

AuxData::AuxData(MooseSystem & moose_system, ElementData & element_data)
  :_moose_system(moose_system),
   _element_data(element_data)
{}
