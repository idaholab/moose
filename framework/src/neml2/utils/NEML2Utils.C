//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2Utils.h"

#ifdef NEML2_ENABLED
#include "ConsoleStream.h"
#include "neml2/csrc/aoti/log.h"
#include <torch/script.h> // torch::jit::Module (TorchScript serialization)
#include <cctype>
#endif

namespace NEML2Utils
{
#ifdef NEML2_ENABLED

void
dumpInputsToTorchScript(const std::map<std::string, at::Tensor> & inputs,
                        const std::string & filename)
{
  // Stash each defined input tensor (moved to host) as a buffer on a TorchScript module, then save.
  // Buffer names must be valid identifiers, so sanitize '~'/'/' etc. to '_'.
  torch::jit::Module mod("neml2_failed_inputs");
  for (const auto & [name, tensor] : inputs)
  {
    if (!tensor.defined())
      continue;
    std::string key = name;
    for (auto & c : key)
      if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
        c = '_';
    mod.register_buffer(key, tensor.detach().to(at::kCPU).contiguous());
  }
  mod.save(filename);
}

void
redirectLogsToConsole(const ConsoleStream & console)
{
  // Capture a copy of the ConsoleStream (it binds the app-owned OutputWarehouse by reference, so
  // the copy stays valid for the app's lifetime) and hand every NEML2 log line to it. The line
  // already carries the [neml2:<channel>] prefix; NEML2's verbosity levels are left at their
  // defaults.
  neml2::aoti::log::set_sink([console](neml2::aoti::log::Level, const std::string & line)
                             { console << line << std::endl; });
}

std::string
stringify(MOOSEIOType type)
{
  switch (type)
  {
    case NEML2Utils::MOOSEIOType::TIME:
      return "TIME";
    case NEML2Utils::MOOSEIOType::SCALAR:
      return "SCALAR";
    case NEML2Utils::MOOSEIOType::FUNCTION:
      return "FUNCTION";
    case NEML2Utils::MOOSEIOType::VARIABLE:
      return "VARIABLE";
    case NEML2Utils::MOOSEIOType::MATERIAL:
      return "MATERIAL";
    default:
      mooseError("Unknown MOOSE IO type.");
  }
}
#endif // NEML2_ENABLED

static const std::string missing_neml2 = "The `NEML2` library is required but not enabled. Refer "
                                         "to the documentation for guidance on how to enable it.";

std::string
docstring(const std::string & desc)
{
#ifndef NEML2_ENABLED
  return missing_neml2 + " (Original description: " + desc + ")";
#else
  return desc;
#endif
}

void
assertNEML2Enabled()
{
#ifndef NEML2_ENABLED
  mooseError(missing_neml2);
#endif
}

} // namespace NEML2Utils
