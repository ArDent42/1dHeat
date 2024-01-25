
#pragma once
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "common.h"
#include "fuel.h"
#include "material.h"
#include "ini_data.h"

namespace base {

enum class FileContent { fuel_total_temp, fuel_props, mat_props };

class Database {
 private:
  std::map<std::string, material::Material> materials_;
  std::unique_ptr<fuel::Fuel> fuel_;
  IniData ini_data_;

  nlohmann::json ReadPropsCSV(const std::string &file_name,
                              FileContent file_content) const;

  std::vector<fuel::FuelProp> IndexingFuelPropNames(
      const parsing::ParsedString &parsed_string) const;

  nlohmann::json ProcessFuelTotalTemp(
      const parsing::ParsedFile &parsed_file) const;
  nlohmann::json ProcessFuelProps(const parsing::ParsedFile &parsed_file) const;

  template <typename Container>
  void PrintValue(std::ostream &os, const Container &c, size_t n) const;

 public:
  Database() = default;
  Database(const std::string& name);
  void OpenEntry(const std::string &name, char entry_type);
  void AddFuel(const std::string &fuel_name,
               const std::vector<std::string> &file_names);

  nlohmann::json CreateFuelEntryTemplate() const;
  nlohmann::json CreateMaterialEntryTemplate() const;
  void CreateEntryTemplate(const std::string &name, char entry_type) const;

  const fuel::Fuel& GetFuel(const std::string &fuel_name) const;
  const material::Material& GetMaterial(const std::string &mat_name) const;

  void PrintFuel(const std::string &fuel_name, std::ostream &os) const;
  void PrintMat(const std::string &mat_name, std::ostream &os) const;
};

}  // namespace base
