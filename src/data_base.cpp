#include "data_base.h"

#include "ini_data.h"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace base {

Database::Database(const IniData& ini) : ini_data_(ini) {
  const IniData::Domain &domain = ini_data_.GetDomainSettings();
  for (size_t i = 0; i < domain.mat_names.size(); ++i) {
    material::Material mat(domain.mat_names[i], domain.angles[i]);
    materials_.emplace(domain.mat_names[i], mat);
  }
  fuel_ = std::make_unique<fuel::Fuel>(domain.fuel_name);
}

void Database::OpenEntry(const std::string &name, char entry_type) {
  std::ifstream fin;
  std::string filename = FileName(name, entry_type);
  fin.open(filename);
  if (!fin.is_open()) {
    throw std::logic_error(filename + " not exist.");
  }
  // json_ = std::make_unique<json>(json::parse(fin));
}

json Database::CreateFuelEntryTemplate() const {
  json json = json::object(
      {{"Comments", nullptr},
       {"Properties", json::array()},
       {"Total temperature", json::array({json::array(), json::array()})}});
  return json;
}

json Database::CreateMaterialEntryTemplate() const {
  json json =
      json::object({{"comments", nullptr},
                    {"ro", json::array({json::array(), json::array()})},
                    {"lambda_0", json::array({json::array(), json::array()})},
                    {"lambda_90", json::array({json::array(), json::array()})},
                    {"cp", json::array({json::array(), json::array()})},
                    {"ko", json::array({json::array(), json::array()})},
                    {"ea", json::array({json::array(), json::array()})}});
  return json;
}

void Database::CreateEntryTemplate(const std::string &name,
                                   char entry_type) const {
  std::ofstream fout(FileName(name, entry_type));
  if (entry_type == 'f') {
    fout << CreateFuelEntryTemplate().dump(4);
  } else if (entry_type == 'm') {
    fout << CreateMaterialEntryTemplate().dump(4);
  }
}

std::vector<fuel::FuelProp> Database::IndexingFuelPropNames(
    const parsing::ParsedString &parsed_string) const {
  int i = 0;
  std::vector<fuel::FuelProp> index_;
  index_.resize(parsed_string.size(), fuel::FuelProp::null);
  for (const auto &param : parsed_string) {
    if (!param) {
      continue;
    }
    if (param.value_or("") == "p") index_[i] = fuel::FuelProp::p;
    if (param.value_or("") == "T") index_[i] = fuel::FuelProp::t;
    if (param.value_or("") == "v") index_[i] = fuel::FuelProp::v;
    if (param.value_or("") == "M") index_[i] = fuel::FuelProp::m;
    if (param.value_or("") == "Cp") index_[i] = fuel::FuelProp::cp_fr;
    if (param.value_or("") == "k") index_[i] = fuel::FuelProp::k_fr;
    if (param.value_or("") == "Cp'") index_[i] = fuel::FuelProp::cp_eq;
    if (param.value_or("") == "k'") index_[i] = fuel::FuelProp::k_eq;
    if (param.value_or("") == "Cpg") index_[i] = fuel::FuelProp::cp_gas_fr;
    if (param.value_or("") == "kg") index_[i] = fuel::FuelProp::k_gas_fr;
    if (param.value_or("") == "Cp'g") index_[i] = fuel::FuelProp::cp_gas_eq;
    if (param.value_or("") == "k'g") index_[i] = fuel::FuelProp::k_gas_eq;
    if (param.value_or("") == "Mu") index_[i] = fuel::FuelProp::mu;
    if (param.value_or("") == "Lt") index_[i] = fuel::FuelProp::lt_gas;
    if (param.value_or("") == "Lt'") index_[i] = fuel::FuelProp::lt_total;
    if (param.value_or("") == "Pr") index_[i] = fuel::FuelProp::pr_fr;
    if (param.value_or("") == "Pr'") index_[i] = fuel::FuelProp::pr_eq;
    if (param.value_or("") == "A") index_[i] = fuel::FuelProp::a;
    if (param.value_or("") == "z") index_[i] = fuel::FuelProp::z;
    if (param.value_or("") == "Bm") index_[i] = fuel::FuelProp::bm;
    if (param.value_or("") == "O") index_[i] = fuel::FuelProp::m_frac_o;
    if (param.value_or("") == "O2") index_[i] = fuel::FuelProp::m_frac_o2;
    if (param.value_or("") == "H2O") index_[i] = fuel::FuelProp::m_frac_h2o;
    if (param.value_or("") == "CO2") index_[i] = fuel::FuelProp::m_frac_co2;
    if (param.value_or("") == "N") index_[i] = fuel::FuelProp::m_frac_n;
    ++i;
  }
  return index_;
}

json Database::ReadPropsCSV(const std::string &file_name,
                            FileContent file_content) const {
  std::ifstream file_in(file_name);
  if (file_in.fail()) {
    throw std::runtime_error(file_name + ": not exist");
  }
  parsing::ParsedFile parsed_file;
  json res;
  parsed_file = parsing::ParsingFuelCSV(file_name);
  if (file_content == FileContent::fuel_total_temp) {
    res = ProcessFuelTotalTemp(parsed_file);
  } else {
    res = ProcessFuelProps(parsed_file);
  }
  return res;
}

json Database::ProcessFuelTotalTemp(
    const parsing::ParsedFile &parsed_file) const {
  json res = json::array({json::array(), json::array()});
  for (size_t i = 1; i < parsed_file.size(); ++i) {
    res[1].push_back(std::stod(parsed_file[i][1].value()));
    res[0].push_back(std::stod(parsed_file[i][0].value()) * math::MEGA);
  }
  return res;
}

json Database::ProcessFuelProps(const parsing::ParsedFile &parsed_file) const {
  json res = json::array();
  std::vector<fuel::FuelProp> prop_index =
      IndexingFuelPropNames(parsed_file.front());
  for (size_t i = 1; i < parsed_file.size(); ++i) {
    json prop_values = json::object({});
    for (size_t j = 0; j < parsed_file[i].size(); ++j) {
      if (prop_index[j] != fuel::FuelProp::null) {
        double prop;
        if (prop_index[j] == fuel::FuelProp::cp_fr ||
            prop_index[j] == fuel::FuelProp::cp_eq ||
            prop_index[j] == fuel::FuelProp::cp_gas_eq ||
            prop_index[j] == fuel::FuelProp::cp_gas_fr) {
          prop = std::stod(parsed_file[i][j].value()) * 1000;
        } else if (prop_index[j] == fuel::FuelProp::p) {
          prop = std::stod(parsed_file[i][j].value()) * math::MEGA;
        } else {
          prop = std::stod(parsed_file[i][j].value());
        }
        prop_values[std::to_string(prop_index[j])] = prop;
      }
    }
    res.push_back(prop_values);
  }
  return res;
}

void Database::AddFuel(const std::string &fuel_name,
                       const std::vector<std::string> &file_names) {
  json res = CreateFuelEntryTemplate();
  res.at("Total temperature") =
      ReadPropsCSV(file_names[0], FileContent::fuel_total_temp);
  for (size_t i = 1; i < file_names.size(); ++i) {
    json array = ReadPropsCSV(file_names[i], FileContent::fuel_props);
    res.at("Properties")
        .insert(res.at("Properties").end(), array.begin(), array.end());
  }
  std::ofstream fout("fuels/" + fuel_name + ".json");
  fout << res.dump(4);
}

const fuel::Fuel &Database::GetFuel(const std::string &fuel_name) const {
  return *fuel_;
}

const material::Material &Database::GetMaterial(
    const std::string &mat_name) const {
  return materials_.at(mat_name);
}

void Database::PrintFuel(const std::string &fuel_name, std::ostream &os) const {
  fuel::Fuel fuel = GetFuel(fuel_name);
  os.setf(std::ios_base::left);
  os << std::setw(8) << "P, MPa" << std::setw(7) << "T, K" << std::endl;
  for (const auto [press, temp] : fuel.GetTotalTemperatures()) {
    os.precision(1);
    os << std::setw(8) << std::fixed << press / 1000000 << std::setw(7) << temp
       << std::endl;
  }
  os << std::endl;
  for (const std::string &name : fuel::PROP_NAMES) {
    os << std::setw(18) << name;
  }
  os << std::endl;
  for (const auto &[press, properties] : fuel.GetProperties()) {
    for (const auto &[temp, props] : properties) {
      os.precision(1);
      os << std::fixed << std::setw(18) << press / 1000000 << std::setw(18)
         << temp;
      os.precision(4);
      for (const auto &[type, prop] : props) {
        os << std::setw(18) << std::scientific << prop;
      }
      os << std::endl;
    }
  }
}

template <typename Container>
void Database::PrintValue(std::ostream &os, const Container &c,
                          size_t n) const {
  int wt = 8;
  int wp = 18;
  auto it = std::next(c.begin(), n);
  if (n < c.size()) {
    os.precision(1);
    os << std::setw(wt) << std::fixed << it->first;
    os.precision(4);
    os << std::setw(wp) << std::scientific << it->second;
  } else {
    os << std::setw(wt) << ' ' << std::setw(wp) << ' ';
  }
}

void Database::PrintMat(const std::string &mat_name, std::ostream &os) const {
  os.setf(std::ios_base::left);
  int wt = 8;
  int wp = 18;
  for (std::string_view name : material::PROP_NAMES) {
    os << std::setw(wt) << "T, K" << std::setw(wp) << name;
  }
  os << std::endl;
  material::Thermal properties_ = GetMaterial(mat_name).GetProps();
  size_t max_size = std::max({properties_.at(material::Property::ro).size(),
                              properties_.at(material::Property::l0).size(),
                              properties_.at(material::Property::l90).size(),
                              properties_.at(material::Property::cp).size(),
                              properties_.at(material::Property::ko).size(),
                              properties_.at(material::Property::ea).size()});
  for (size_t i = 0; i < max_size; ++i) {
    PrintValue(os, properties_.at(material::Property::ro), i);
    PrintValue(os, properties_.at(material::Property::l0), i);
    PrintValue(os, properties_.at(material::Property::l90), i);
    PrintValue(os, properties_.at(material::Property::cp), i);
    PrintValue(os, properties_.at(material::Property::ko), i);
    PrintValue(os, properties_.at(material::Property::ea), i);
    os << std::endl;
  }
}

}  // namespace base
