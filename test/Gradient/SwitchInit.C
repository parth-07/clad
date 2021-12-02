// RUN: %cladclang %s -I%S/../../include -std=c++17 -oSwitchInit.out 2>&1 -lstdc++ -lm | FileCheck %s
// RUN: ./SwitchInit.out | FileCheck -check-prefix=CHECK-EXEC %s
//CHECK-NOT: {{.*error|warning|note:.*}}

#include "clad/Differentiator/Differentiator.h"

double fn1(double i, double j) {
  double res = 0;
  switch (int count = 1;count) {
    case 0: res += i * j; break;
    case 1: res += i * i; {
        case 2: res += j * j;
      }
    default: res += i * i * j * j;
  }
  return res;
}

// CHECK: void fn1_grad(double i, double j, clad::array_ref<double> _d_i, clad::array_ref<double> _d_j) {
// CHECK-NEXT:     double _d_res = 0;
// CHECK-NEXT:     int _d_count = 0;
// CHECK-NEXT:     int _cond0;
// CHECK-NEXT:     double _t0;
// CHECK-NEXT:     double _t1;
// CHECK-NEXT:     clad::tape<unsigned long> _t2 = {};
// CHECK-NEXT:     double _t3;
// CHECK-NEXT:     double _t4;
// CHECK-NEXT:     double _t5;
// CHECK-NEXT:     double _t6;
// CHECK-NEXT:     double _t7;
// CHECK-NEXT:     double _t8;
// CHECK-NEXT:     double _t9;
// CHECK-NEXT:     double _t10;
// CHECK-NEXT:     double _t11;
// CHECK-NEXT:     double _t12;
// CHECK-NEXT:     double res = 0;
// CHECK-NEXT:     {
// CHECK-NEXT:         int count = 1;
// CHECK-NEXT:         _cond0 = count;
// CHECK-NEXT:         switch (_cond0) {
// CHECK-NEXT:             {
// CHECK-NEXT:               case 0:
// CHECK-NEXT:                 res += _t1 * _t0;
// CHECK-NEXT:                 _t1 = i;
// CHECK-NEXT:                 _t0 = j;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 clad::push(_t2, 1UL);
// CHECK-NEXT:                 break;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:               case 1:
// CHECK-NEXT:                 res += _t4 * _t3;
// CHECK-NEXT:                 _t4 = i;
// CHECK-NEXT:                 _t3 = i;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                   case 2:
// CHECK-NEXT:                     res += _t6 * _t5;
// CHECK-NEXT:                     _t6 = j;
// CHECK-NEXT:                     _t5 = j;
// CHECK-NEXT:                 }
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:               default:
// CHECK-NEXT:                 res += _t12 * _t7;
// CHECK-NEXT:                 _t10 = i;
// CHECK-NEXT:                 _t9 = i;
// CHECK-NEXT:                 _t11 = _t10 * _t9;
// CHECK-NEXT:                 _t8 = j;
// CHECK-NEXT:                 _t12 = _t11 * _t8;
// CHECK-NEXT:                 _t7 = j;
// CHECK-NEXT:             }
// CHECK-NEXT:             clad::push(_t2, 2UL);
// CHECK-NEXT:         }
// CHECK-NEXT:     }
// CHECK-NEXT:     double fn1_return = res;
// CHECK-NEXT:     goto _label0;
// CHECK-NEXT:   _label0:
// CHECK-NEXT:     _d_res += 1;
// CHECK-NEXT:     {
// CHECK-NEXT:         switch (clad::pop(_t2)) {
// CHECK-NEXT:           case 2UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     double _r_d3 = _d_res;
// CHECK-NEXT:                     _d_res += _r_d3;
// CHECK-NEXT:                     double _r6 = _r_d3 * _t7;
// CHECK-NEXT:                     double _r7 = _r6 * _t8;
// CHECK-NEXT:                     double _r8 = _r7 * _t9;
// CHECK-NEXT:                     * _d_i += _r8;
// CHECK-NEXT:                     double _r9 = _t10 * _r7;
// CHECK-NEXT:                     * _d_i += _r9;
// CHECK-NEXT:                     double _r10 = _t11 * _r6;
// CHECK-NEXT:                     * _d_j += _r10;
// CHECK-NEXT:                     double _r11 = _t12 * _r_d3;
// CHECK-NEXT:                     * _d_j += _r11;
// CHECK-NEXT:                     _d_res -= _r_d3;
// CHECK-NEXT:                 }
// CHECK-NEXT:                 if (_cond0 != 0 && _cond0 != 1 && _cond0 != 2)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     {
// CHECK-NEXT:                         double _r_d2 = _d_res;
// CHECK-NEXT:                         _d_res += _r_d2;
// CHECK-NEXT:                         double _r4 = _r_d2 * _t5;
// CHECK-NEXT:                         * _d_j += _r4;
// CHECK-NEXT:                         double _r5 = _t6 * _r_d2;
// CHECK-NEXT:                         * _d_j += _r5;
// CHECK-NEXT:                         _d_res -= _r_d2;
// CHECK-NEXT:                     }
// CHECK-NEXT:                     if (2 == _cond0)
// CHECK-NEXT:                         break;
// CHECK-NEXT:                 }
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     double _r_d1 = _d_res;
// CHECK-NEXT:                     _d_res += _r_d1;
// CHECK-NEXT:                     double _r2 = _r_d1 * _t3;
// CHECK-NEXT:                     * _d_i += _r2;
// CHECK-NEXT:                     double _r3 = _t4 * _r_d1;
// CHECK-NEXT:                     * _d_i += _r3;
// CHECK-NEXT:                     _d_res -= _r_d1;
// CHECK-NEXT:                 }
// CHECK-NEXT:                 if (1 == _cond0)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:           case 1UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     double _r_d0 = _d_res;
// CHECK-NEXT:                     _d_res += _r_d0;
// CHECK-NEXT:                     double _r0 = _r_d0 * _t0;
// CHECK-NEXT:                     * _d_i += _r0;
// CHECK-NEXT:                     double _r1 = _t1 * _r_d0;
// CHECK-NEXT:                     * _d_j += _r1;
// CHECK-NEXT:                     _d_res -= _r_d0;
// CHECK-NEXT:                 }
// CHECK-NEXT:                 if (0 == _cond0)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:         }
// CHECK-NEXT:     }
// CHECK-NEXT: }

#define TEST_2(F, x, y)                                                        \
  {                                                                            \
    result[0] = result[1] = 0;                                                 \
    auto d_##F = clad::gradient(F);                                            \
    d_##F.execute(x, y, result, result + 1);                                   \
    printf("{%.2f, %.2f}\n", result[0], result[1]);                            \
  }

int main() {
  double result[2] = {};
  clad::array_ref<double> result_ref(result, 2);

  TEST_2(fn1, 3, 5); // CHECK-EXEC: {156.00, 100.00}
}