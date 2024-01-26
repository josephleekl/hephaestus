#pragma once
#include "kernel_base.hpp"

namespace hephaestus
{

/*
(βu, u')
*/
class VectorFEMassKernel : public Kernel<mfem::ParBilinearForm>
{
public:
  VectorFEMassKernel(const hephaestus::InputParameters & params);

  ~VectorFEMassKernel() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients) override;
  void Apply(mfem::ParBilinearForm * blf) override;
  std::string _coef_name;
  std::shared_ptr<mfem::Coefficient> _coef{nullptr};
};

} // namespace hephaestus
