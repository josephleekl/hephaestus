#include "auxsolvers.hpp"

namespace hephaestus {

void AuxSolvers::Init(
    const mfem::NamedFieldsMap<mfem::ParGridFunction> &variables,
    hephaestus::Coefficients &coefficients) {

  for (const auto &[name, auxsolver] : GetMap()) {
    auxsolver->Init(variables, coefficients);
    aux_queue.push_back(auxsolver);
  }
  std::sort(aux_queue.begin(), aux_queue.end(), AuxCompare());
}

void AuxSolvers::Solve(double t) {
  for (auto &auxsolver : aux_queue) {
    auxsolver->Solve(t);
  }
}

} // namespace hephaestus
