//--------------------------------------------------------------------*- C++ -*-
// clad - the C++ Clang-based Automatic Differentiator
// version: $Id$
// author:  Vassil Vassilev <vvasilev-at-cern.ch>
//------------------------------------------------------------------------------

#ifndef CLAD_BUILTIN_DERIVATIVES
#define CLAD_BUILTIN_DERIVATIVES

// Avoid assertion custom_derivative namespace not found. FIXME: This in future
// should go.
namespace custom_derivatives{}

#include "clad/Differentiator/ArrayRef.h"
#include "clad/Differentiator/CladConfig.h"

#include <cmath>
namespace clad {
namespace custom_derivatives {
namespace std {
template <typename T> CUDA_HOST_DEVICE T abs_pushforward(T x, T d_x) {
  if (x >= 0)
    return d_x;
  else
    return -d_x;
}

template <typename T> CUDA_HOST_DEVICE T exp_pushforward(T x, T d_x) {
  return ::std::exp(x) * d_x;
}

template <typename T>
CUDA_HOST_DEVICE T exp_pushforward_pushforward(T x, T d_x, T d_x0, T d_d_x) {
  return ::std::exp(x) * d_x0 * d_x + ::std::exp(x) * d_d_x;
}

template <typename T> CUDA_HOST_DEVICE T sin_pushforward(T x, T d_x) {
  return ::std::cos(x) * d_x;
}

template <typename T> CUDA_HOST_DEVICE T cos_pushforward(T x, T d_x) {
  return (-1) * ::std::sin(x) * d_x;
}

template <typename T>
CUDA_HOST_DEVICE T sin_pushforward_pushforward(T x, T d_x, T d_x0, T d_d_x) {
  return cos_pushforward(x, d_x0) * d_x + ::std::cos(x) * d_d_x;
}

template <typename T>
CUDA_HOST_DEVICE T cos_pushforward_pushforward(T x, T d_x, T d_x0, T d_d_x) {
  return (-1) * (sin_pushforward(x, d_x0) * d_x + ::std::sin(x) * d_d_x);
}

template <typename T> CUDA_HOST_DEVICE T sqrt_pushforward(T x, T d_x) {
  return (((T)1) / (((T)2) * ::std::sqrt(x))) * d_x;
}

#ifdef MACOS
float sqrtf_pushforward(float x, float d_x) {
  return (1.F / (2.F * sqrtf(x))) * d_x;
}

#endif

template <typename T1, typename T2>
CUDA_HOST_DEVICE decltype(::std::pow(T1(), T2()))
pow_pushforward(T1 x, T2 exponent, T1 d_x, T2 d_exponent) {
  return (exponent * ::std::pow(x, exponent - 1)) * d_x +
         (::std::pow(x, exponent) * ::std::log(x)) * d_exponent;
}

template <typename T> CUDA_HOST_DEVICE T log_pushforward(T x, T d_x) {
  return (1.0 / x) * d_x;
}

template <typename T1, typename T2>
CUDA_HOST_DEVICE void
pow_pullback(T1 x, T2 exponent, decltype(::std::pow(T1(), T2())) d_y,
             clad::array_ref<decltype(::std::pow(T1(), T2()))> d_x,
             clad::array_ref<decltype(::std::pow(T1(), T2()))> d_exponent) {
  *d_x += pow_pushforward(x, exponent, static_cast<T1>(1), static_cast<T2>(0)) *
          d_y;
  *d_exponent +=
      pow_pushforward(x, exponent, static_cast<T1>(0), static_cast<T2>(1)) *
      d_y;
}
} // namespace std
// These are required because C variants of mathematical functions are
// defined in global namespace.
using std::abs_pushforward;
using std::cos_pushforward;
using std::cos_pushforward_pushforward;
using std::exp_pushforward;
using std::exp_pushforward_pushforward;
using std::log_pushforward;
using std::pow_pushforward;
using std::sin_pushforward;
using std::sin_pushforward_pushforward;
using std::sqrt_pushforward;
using std::pow_pullback;
} // namespace custom_derivatives
} // namespace clad

  // FIXME: These math functions depend on promote_2 just like pow:
  // atan2
  // fmod
  // copysign
  // fdim
  // fmax
  // fmin
  // hypot
  // nextafter
  // remainder
  // remquo
#endif //CLAD_BUILTIN_DERIVATIVES
