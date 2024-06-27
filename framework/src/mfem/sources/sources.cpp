#include "sources.hpp"

namespace hephaestus
{

void
Sources::Init(hephaestus::GridFunctions & gridfunctions,
              const hephaestus::FESpaces & fespaces,
              hephaestus::BCMap & bc_map,
              hephaestus::Coefficients & coefficients)
{
  for (const auto & [name, source] : *this)
  {
    logger.info("Initialising {} Source", name);
    spdlog::stopwatch sw;
    source->Init(gridfunctions, fespaces, bc_map, coefficients);
    logger.info("{} Init: {} seconds", name, sw);
  }
}

void
Sources::Apply(mfem::ParLinearForm * lf)
{
  for (const auto & [name, source] : *this)
  {
    logger.info("Applying {} Source", name);
    spdlog::stopwatch sw;
    source->Apply(lf);
    logger.info("{} Apply: {} seconds", name, sw);
  }
}

void
Sources::SubtractSources(mfem::ParGridFunction * gf)
{
  for (const auto & [name, source] : *this)
  {
    logger.info("Subtracting {} Source", name);
    spdlog::stopwatch sw;
    source->SubtractSource(gf);
    logger.info("{} SubtractSource: {} seconds", name, sw);
  }
}

} // namespace hephaestus
