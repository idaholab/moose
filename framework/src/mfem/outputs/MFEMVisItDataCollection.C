#ifdef MFEM_ENABLED

#include "MFEMVisItDataCollection.h"

registerMooseObject("MooseApp", MFEMVisItDataCollection);

InputParameters
MFEMVisItDataCollection::validParams()
{
  InputParameters params = MFEMDataCollection::validParams();
  params.addClassDescription("Output for controlling export to an mfem::VisItDataCollection.");
  params.addParam<unsigned int>("refinements",
                                0,
                                "Number of uniform refinements for oversampling "
                                "(refinement levels beyond any uniform "
                                "refinements)");
  return params;
}

MFEMVisItDataCollection::MFEMVisItDataCollection(const InputParameters & parameters)
  : MFEMDataCollection(parameters),
    _visit_dc((_file_base + std::string("/Run") + std::to_string(getFileNumber())).c_str(),
              _problem_data.pmesh.get()),
    _refinements(getParam<unsigned int>("refinements"))
{
  _visit_dc.SetLevelsOfDetail(_refinements + 1);
  registerFields();
}

#endif
