#pragma once
#include <map>
#include <set>
#include <string>
#include <memory>
#include <vector>
#include <mfem.hpp>

namespace hephaestus
{

/// Lightweight adaptor over an std::map from strings to pointer to T
template <typename T>
class NamedFieldsMap
{
public:
  using MapType = std::map<std::string, std::shared_ptr<T>>;
  using iterator = typename MapType::iterator;
  using const_iterator = typename MapType::const_iterator;

  /// Default initializer.
  NamedFieldsMap() = default;

  /// Destructor.
  ~NamedFieldsMap() { DeregisterAll(); }

  /// Construct new field with name @field_name and register.
  template <class FieldType, class... FieldArgs>
  void Register(const std::string & field_name, FieldArgs &&... args)
  {
    Register(field_name, std::make_shared<FieldType>(std::forward<FieldArgs>(args)...));
  }

  /// Register association between field @a field and name @a field_name.
  void Register(const std::string & field_name, std::shared_ptr<T> field)
  {
    CheckForDoubleRegistration(field_name, field.get());

    Deregister(field_name);

    _field_map[field_name] = std::move(field);
  }

  /// Unregister association between field @a field and name @a field_name.
  void Deregister(const std::string & field_name) { _field_map.erase(field_name); }

  /// Predicate to check if a field is associated with name @a field_name.
  [[nodiscard]] inline bool Has(const std::string & field_name) const
  {
    return find(field_name) != end();
  }

  /// Get a shared pointer to the field associated with name @a field_name.
  [[nodiscard]] inline std::shared_ptr<T> Get(const std::string & field_name,
                                              bool nullable = true) const
  {
    if (!nullable)
    {
      CheckForFieldRegistration(field_name);
    }

    auto it = find(field_name);
    return it != end() ? it->second : nullptr;
  }

  /// Get a non-owning pointer to the field associated with name @a field_name.
  [[nodiscard]] inline T * GetPtr(const std::string & field_name, bool nullable = true) const
  {
    auto owned_ptr = Get(field_name, nullable);

    return owned_ptr ? owned_ptr.get() : nullptr;
  }

  template <typename TDerived>
  [[nodiscard]] inline TDerived * GetPtr(const std::string & field_name, bool nullable = true) const
  {
    auto ptr = GetPtr(field_name, nullable);

    return ptr ? dynamic_cast<TDerived *>(ptr) : nullptr;
  }

  /// Get a reference to a field.
  [[nodiscard]] inline T & GetRef(const std::string & field_name) const
  {
    return *Get(field_name, false);
  }

  /// Get a shared pointer to the field and cast to subclass TDerived.
  template <typename TDerived>
  [[nodiscard]] inline std::shared_ptr<TDerived> Get(const std::string & field_name) const
  {
    auto owned_ptr = Get(field_name);

    return owned_ptr ? std::dynamic_pointer_cast<TDerived>(owned_ptr) : nullptr;
  }

  /// Returns a vector containing all values for supplied keys.
  [[nodiscard]] std::vector<T *> Get(const std::vector<std::string> keys) const
  {
    std::vector<T *> values;

    for (const auto & key : keys)
    {
      values.push_back(GetPtr(key, false));
    }

    values.shrink_to_fit();
    return values;
  }

  /// Returns reference to field map.
  inline MapType & GetMap() { return _field_map; }

  /// Returns const-reference to field map.
  [[nodiscard]] inline const MapType & GetMap() const { return _field_map; }

  /// Returns a begin iterator to the registered fields.
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline iterator begin() { return _field_map.begin(); }

  /// Returns a begin const iterator to the registered fields.
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] inline const_iterator begin() const { return _field_map.begin(); }

  /// Returns an end iterator to the registered fields.
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline iterator end() { return _field_map.end(); }

  /// Returns an end const iterator to the registered fields.
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] inline const_iterator end() const { return _field_map.end(); }

  /// Returns an iterator to the field @a field_name.
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline iterator find(const std::string & field_name) { return _field_map.find(field_name); }

  /// Returns a const iterator to the field @a field_name.
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] inline const_iterator find(const std::string & field_name) const
  {
    return _field_map.find(field_name);
  }

  /// Returns the number of registered fields.
  [[nodiscard]] inline int NumFields() const { return _field_map.size(); }

protected:
  /// Check for double-registration of a field. A double-registered field may
  /// result in undefined behavior.
  void CheckForDoubleRegistration(const std::string & field_name, T * field) const
  {
    if (Has(field_name) && GetPtr(field_name) == field)
    {
      MFEM_ABORT("The field '" << field_name << "' is already registered.");
    }
  }

  /// Checks that field has been registered.
  void CheckForFieldRegistration(const std::string & field_name) const
  {
    if (!Has(field_name))
    {
      MFEM_ABORT("No field with name '" << field_name << "' has been registered.");
    }
  }

  /// Returns a valid shared pointer to the field with name field_name.
  [[nodiscard]] inline std::shared_ptr<T> GetValid(const std::string & field_name) const
  {
    CheckForFieldRegistration(field_name);
    return Get(field_name);
  }

  /// Clear all associations between names and fields.
  void DeregisterAll() { _field_map.clear(); }

private:
  MapType _field_map{};
};
} // namespace hephaestus