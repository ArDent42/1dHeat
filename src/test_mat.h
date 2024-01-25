#pragma once
#include <cassert>
#include <cmath>

#include "common.h"
#include "material.h"

namespace material {

void TestOpen() {
  Material mat("yt3", 60.0);
  Thermal props = mat.GetProps();
  assert(props.size() == 7);
  assert(props.at(Property::ro).size() == 1);
  assert(props.at(Property::l0).size() == 14);
  assert(props.at(Property::l90).size() == 14);
  assert(props.at(Property::l).size() == 14);
  assert(props.at(Property::cp).size() == 14);
  assert(props.at(Property::ko).size() == 1);
  assert(props.at(Property::ea).size() == 1);
  std::cout << "TestOpen are OK" << std::endl;
}

void TestGetProperty() {
  Material mat("yt3", 60.0);
  for(size_t i = 0; i < 3; ++i) {
    assert(std::abs(mat.GetProperty(273.0 * i, Property::ro) - 600.0) < EPS);
    assert(std::abs(mat.GetProperty(273.0 * i, Property::ko) - 40000.0) < EPS);
    assert(std::abs(mat.GetProperty(273.0 * i, Property::ea) - 1.71E06) < EPS);
  }
  assert(mat.GetProperty(250.0, Property::l) > 0.6 &&
         mat.GetProperty(250.0, Property::l) < 1.060);
  assert(mat.GetProperty(3400.0, Property::l) > 7.5 &&
         mat.GetProperty(3400.0, Property::l) < 17.0);
  assert(std::abs(mat.GetProperty(250.0, Property::cp) - 930.0) < EPS);
  assert(std::abs(mat.GetProperty(3400.0, Property::cp) - 2180.0) < EPS);
  assert(std::abs(mat.GetProperty(900.0, Property::cp) - (1780.0 + 1845.0) / 2.0) < EPS);
  std::cout << "TestGetProperty are OK" << std::endl;
}

}  // namespace material