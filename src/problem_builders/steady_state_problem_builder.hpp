#pragma once
#include "problem_operator.hpp"
#include "problem_builder_base.hpp"
namespace hephaestus
{

class SteadyStateProblem : public hephaestus::Problem
{
public:
  friend class SteadyStateProblemBuilder;

  SteadyStateProblem() = default;
  ~SteadyStateProblem() override = default;

  [[nodiscard]] bool HasEquationSystem() const override
  {
    return GetOperator()->HasEquationSystem();
  }

  [[nodiscard]] hephaestus::EquationSystem * GetEquationSystem() const override
  {
    return GetOperator()->GetEquationSystem();
  }

  [[nodiscard]] hephaestus::ProblemOperator * GetOperator() const override
  {
    if (!_ss_operator)
    {
      MFEM_ABORT("No ProblemOperator has been added to SteadyStateProblem.");
    }

    return _ss_operator.get();
  }

  void SetOperator(std::unique_ptr<ProblemOperator> new_problem_operator)
  {
    _ss_operator.reset();
    _ss_operator = std::move(new_problem_operator);
  }

protected:
  std::unique_ptr<hephaestus::ProblemOperator> _ss_operator{nullptr};
};

// Builder class of a frequency-domain problem.
class SteadyStateProblemBuilder : public hephaestus::ProblemBuilder
{
public:
  SteadyStateProblemBuilder() : _problem(std::make_unique<hephaestus::SteadyStateProblem>()) {}

  ~SteadyStateProblemBuilder() override = default;

  virtual std::unique_ptr<hephaestus::SteadyStateProblem> ReturnProblem()
  {
    return std::move(_problem);
  }

  void RegisterFESpaces() override {}

  void RegisterGridFunctions() override {}

  void RegisterAuxSolvers() override {}

  void RegisterCoefficients() override {}

  void InitializeKernels() override;

  void ConstructEquationSystem() override;

  void SetOperatorGridFunctions() override;

  void ConstructOperator() override;

  void ConstructState() override;

  void ConstructTimestepper() override {}

protected:
  std::unique_ptr<hephaestus::SteadyStateProblem> _problem{nullptr};
  mfem::ConstantCoefficient _one_coef{1.0};

  hephaestus::SteadyStateProblem * GetProblem() override { return _problem.get(); };
};

} // namespace hephaestus
