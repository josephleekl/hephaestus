#pragma once
#include "../common/pfem_extras.hpp"
#include "hcurl_solver.hpp"
#include "inputs.hpp"

namespace hephaestus {

class HFormulation : public hephaestus::HCurlFormulation {
public:
  HFormulation();

  ~HFormulation(){};

  virtual void
  RegisterAuxKernels(mfem::NamedFieldsMap<mfem::ParGridFunction> &variables,
                     hephaestus::AuxKernels &auxkernels) override;

  virtual void RegisterCoefficients(
      hephaestus::DomainProperties &domain_properties) override;
};
} // namespace hephaestus
