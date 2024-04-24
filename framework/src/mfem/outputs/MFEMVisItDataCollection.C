#include "MFEMVisItDataCollection.h"

registerMooseObject("PlatypusApp", MFEMVisItDataCollection);

InputParameters
MFEMVisItDataCollection::validParams()
{
  InputParameters params = MFEMDataCollection::validParams();
  params.addClassDescription("Output for controlling MFEMVisItDataCollection inherited data.");
  params.addParam<unsigned int>("refinements",
                                0,
                                "Number of uniform refinements for oversampling "
                                "(refinement levels beyond any uniform "
                                "refinements)");
  return params;
}

MFEMVisItDataCollection::MFEMVisItDataCollection(const InputParameters & parameters)
  : MFEMDataCollection(parameters), _refinements(getParam<unsigned int>("refinements"))
{
}

std::shared_ptr<mfem::DataCollection>
MFEMVisItDataCollection::createDataCollection(const std::string & collection_name) const
{
  auto visit_dc = std::make_shared<mfem::VisItDataCollection>(_file_base.c_str() + collection_name);
  visit_dc->SetLevelsOfDetail(_refinements + 1);

  return visit_dc;
}