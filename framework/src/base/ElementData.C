//Moose includes
#include "ElementData.h"
#include "MooseSystem.h"

//libmesh includes

ElementData::ElementData(MooseSystem & moose_system)
  :_moose_system(moose_system)
{}
