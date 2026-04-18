//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVisItDataCollection.h"

registerMooseMFEMObject("MooseApp", VisItDataCollection);

namespace Moose::MFEM
{
InputParameters
VisItDataCollection::validParams()
{
  InputParameters params = DataCollection::validParams();
  params.addClassDescription("Output for controlling export to an mfem::VisItDataCollection.");
  params.addParam<unsigned int>("refinements",
                                0,
                                "Number of uniform refinements for oversampling "
                                "(refinement levels beyond any uniform "
                                "refinements)");
  return params;
}

VisItDataCollection::VisItDataCollection(const InputParameters & parameters)
  : DataCollection(parameters),
    _visit_dc((_file_base + std::string("/Run") + std::to_string(getFileNumber())).c_str(),
              &_pmesh),
    _refinements(getParam<unsigned int>("refinements"))
{
  _visit_dc.SetLevelsOfDetail(_refinements + 1);
  registerFields();
}

} // namespace Moose::MFEM
#endif
