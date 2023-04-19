#pragma once
#include <fstream>
#include <iostream>
#include <memory>

#include "mesh_extras.hpp"

namespace hephaestus {

class BoundaryCondition {
public:
  BoundaryCondition();
  BoundaryCondition(const std::string &name_, mfem::Array<int> bdr_attributes_);
  mfem::Array<int> getMarkers(mfem::Mesh &mesh);

  std::string name;
  mfem::Array<int> bdr_attributes;
  mfem::Array<int> markers;

  virtual void applyBC(mfem::LinearForm &b){};
  virtual void applyBC(mfem::ComplexLinearForm &b){};
  virtual void applyBC(mfem::ParComplexLinearForm &b){};
};

class EssentialBC : virtual public BoundaryCondition {
public:
  EssentialBC();
  EssentialBC(const std::string &name_, mfem::Array<int> bdr_attributes_);

  virtual void applyBC(mfem::GridFunction &gridfunc, mfem::Mesh *mesh_){};
  virtual void applyBC(mfem::ParComplexGridFunction &gridfunc,
                       mfem::Mesh *mesh_){};
};

class FunctionDirichletBC : public EssentialBC {
public:
  FunctionDirichletBC();
  FunctionDirichletBC(const std::string &name_,
                      mfem::Array<int> bdr_attributes_);
  FunctionDirichletBC(const std::string &name_,
                      mfem::Array<int> bdr_attributes_,
                      mfem::FunctionCoefficient *coeff_,
                      mfem::FunctionCoefficient *coeff_im_ = nullptr);

  virtual void applyBC(mfem::GridFunction &gridfunc,
                       mfem::Mesh *mesh_) override;

  mfem::FunctionCoefficient *coeff;
  mfem::FunctionCoefficient *coeff_im;
};

class VectorFunctionDirichletBC : virtual public EssentialBC {
public:
  VectorFunctionDirichletBC();
  VectorFunctionDirichletBC(const std::string &name_,
                            mfem::Array<int> bdr_attributes_);
  VectorFunctionDirichletBC(
      const std::string &name_, mfem::Array<int> bdr_attributes_,
      mfem::VectorFunctionCoefficient *vec_coeff_,
      mfem::VectorFunctionCoefficient *vec_coeff_im_ = nullptr);

  virtual void applyBC(mfem::GridFunction &gridfunc,
                       mfem::Mesh *mesh_) override;

  virtual void applyBC(mfem::ParComplexGridFunction &gridfunc,
                       mfem::Mesh *mesh_) override;

  mfem::VectorFunctionCoefficient *vec_coeff;
  mfem::VectorFunctionCoefficient *vec_coeff_im;
};

class IntegratedBC : virtual public BoundaryCondition {
public:
  IntegratedBC();
  IntegratedBC(const std::string &name_, mfem::Array<int> bdr_attributes_);
  IntegratedBC(const std::string &name_, mfem::Array<int> bdr_attributes_,
               mfem::LinearFormIntegrator *lfi_re_,
               mfem::LinearFormIntegrator *lfi_im_ = nullptr);

  mfem::LinearFormIntegrator *lfi_re;
  mfem::LinearFormIntegrator *lfi_im;

  virtual void applyBC(mfem::LinearForm &b) override;
  virtual void applyBC(mfem::ComplexLinearForm &b) override;
  virtual void applyBC(mfem::ParComplexLinearForm &b) override;
};

class RobinBC : public EssentialBC, IntegratedBC {
public:
  RobinBC();
  RobinBC(const std::string &name_, mfem::Array<int> bdr_attributes_);
  RobinBC(const std::string &name_, mfem::Array<int> bdr_attributes_,
          mfem::Coefficient *robin_coeff_, mfem::LinearFormIntegrator *lfi_re_,
          mfem::Coefficient *robin_coeff_im = nullptr,
          mfem::LinearFormIntegrator *lfi_im_ = nullptr);

  mfem::Coefficient *robin_coeff_re;
  mfem::Coefficient *robin_coeff_im;
};

class VectorRobinBC : public VectorFunctionDirichletBC, IntegratedBC {
public:
  VectorRobinBC(const std::string &name_, mfem::Array<int> bdr_attributes_,
                mfem::BilinearFormIntegrator *blfi_re_,
                mfem::VectorFunctionCoefficient *vec_coeff_re_,
                mfem::LinearFormIntegrator *lfi_re_,
                mfem::BilinearFormIntegrator *blfi_im_ = nullptr,
                mfem::VectorFunctionCoefficient *vec_coeff_im_ = nullptr,
                mfem::LinearFormIntegrator *lfi_im_ = nullptr);

  mfem::BilinearFormIntegrator *blfi_re;
  mfem::BilinearFormIntegrator *blfi_im;

  virtual void applyBC(mfem::ParSesquilinearForm &a);
};

class BCMap : public std::map<std::string, hephaestus::BoundaryCondition *> {
public:
  mfem::Array<int> getEssentialBdrMarkers(const std::string &name_,
                                          mfem::Mesh *mesh_);

  void applyEssentialBCs(const std::string &name_,
                         mfem::Array<int> &ess_tdof_list,
                         mfem::GridFunction &gridfunc, mfem::Mesh *mesh_);

  void applyEssentialBCs(const std::string &name_,
                         mfem::Array<int> &ess_tdof_list,
                         mfem::ParComplexGridFunction &gridfunc,
                         mfem::Mesh *mesh_);

  void applyIntegratedBCs(const std::string &name_, mfem::LinearForm &lf,
                          mfem::Mesh *mesh_);

  void applyIntegratedBCs(const std::string &name_,
                          mfem::ParComplexLinearForm &clf, mfem::Mesh *mesh_);
};

} // namespace hephaestus
