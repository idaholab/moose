#ifdef MFEM_ENABLED

#include "MFEMParaViewDataCollection.h"

registerMooseObject("MooseApp", MFEMParaViewDataCollection);

InputParameters
MFEMParaViewDataCollection::validParams()
{
  InputParameters params = MFEMDataCollection::validParams();
  params.addClassDescription("Output for controlling export to an mfem::ParaViewDataCollection.");
  params.addParam<unsigned int>("refinements",
                                0,
                                "Number of uniform refinements for oversampling "
                                "(refinement levels beyond any uniform "
                                "refinements)");
  params.addParam<bool>("high_order_output",
                        false,
                        "Sets whether or not to output the data as "
                        "high-order elements (false by default)."
                        "Reading high-order data requires ParaView"
                        "5.5 or later.");

  MooseEnum vtk_format("ASCII BINARY BINARY32", "BINARY", true);
  params.addParam<MooseEnum>(
      "vtk_format",
      vtk_format,
      "Select VTK data format to use, choosing between BINARY, BINARY32, and ASCII.");
  return params;
}

MFEMParaViewDataCollection::MFEMParaViewDataCollection(const InputParameters & parameters)
  : MFEMDataCollection(parameters),
    _pv_dc((_file_base + std::string("/Run") + std::to_string(getFileNumber())).c_str(), &_pmesh),
    _high_order_output(getParam<bool>("high_order_output")),
    _refinements(getParam<unsigned int>("refinements")),
    _vtk_format(parameters.get<MooseEnum>("vtk_format").getEnum<mfem::VTKFormat>())
{
  _pv_dc.SetPrecision(9);
  _pv_dc.SetHighOrderOutput(_high_order_output);
  _pv_dc.SetLevelsOfDetail(_refinements + 1);
  _pv_dc.SetDataFormat(_vtk_format);
  registerFields();
}

#endif
