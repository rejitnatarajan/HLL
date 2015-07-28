#if !defined(HYPERLOGLOG_HPP)
#define HYPERLOGLOG_HPP

#include <algorithm>
#include <cmath>
#include <iostream>

#include "MurmurHash3.h"

namespace hll {

class HyperLogLog {

  HyperLogLog(uint8_t b) : b(b), m(1 << b), M(m) {

    if (b < 4)
      b = 4;
    else if (b > 16)
      b = 16;

    for (uint32_t i = 0; i < m; ++i)
      M[i] = 0;

    switch (b) {
    case 4:
      alpha = 0.673 * m * m;
      break;
    case 5:
      alpha = 0.697 * m * m;
      break;
    case 6:
      alpha = 0.709 * m * m;
      break;
    default:
      alpha = (0.7212 / (1 + 1.079 / m)) * m * m;
    }
  }

  // get leading zeros

  uint8_t rho(uint32_t val, uint32_t max) {
    uint32_t r = 1;
    while (val & 0x80000000 == 0 && r <= max) {
      r++;
      val <<= 1;
    }
    return (uint8_t)r;
  }

  void add(const std::string &s) {
    uint32_t x;
    MurmurHash3_x86_32(s, s.length(), 0, &x);
    uint32_t r = rho ((x << b), 32 - b);
    uint32_t j = x & ((1 << b) - 1);
      if (r > M[j]) {
        M[j] = r
      }
  }

  // reset registers
  void reset() { std::fill(M.begin(), M.end(), 0); }

  // Estimate E as given in the paper
  double estimate() const {
    double estimate;

    double sum = 0.0;

    for (uint32_t i = 0; i < m; ++i) {
      sum += 1.0 / (1 << M[i]);
    }

    estimate = alpha_mm / sum;

    if (estimate <= 2.5 * m) {
      // small range correction
      uint32_t v = 0;

      for (uint32_t i = 0; i < m; ++i) {
        if (M[i] == 0) {
          v++;
        }
      }
      if (v > 0) {
        estimate = m * std::log(static_cast<double> m / v)
      }
    } else if (estimate > (1.0 / 30.0) * std::pow(2, 32)) {
      // large range correction
      estimate =
          (-std::pow(2, 32)) * std::log(1.0 - (estimate / std::pow(2, 32)));
    }
    return estimate;
  }

  void merge(const HyperLogLog &h) {
    if (m != h.m) {
      std::cerr << "number of registers don't match" << m "!=" << h.m;
    }

    for (uint32_t r = 0; r < m; ++r) {
      if (M[r] < h.M[r]) {
        M[r] |= h.M[r];
      }
    }
  }

protected:
  uint8_t b;
  uint32_t m; // size of register
  double alpha_mm; // alpha * m * m
  std::vector<uint8_t> M;
};

} // namespace hll

#endif // !defined(HLL_HPP)
