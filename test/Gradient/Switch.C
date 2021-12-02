// RUN: %cladclang %s -I%S/../../include -oSwitch.out 2>&1 -lstdc++ -lm | FileCheck %s
// RUN: ./Switch.out | FileCheck -check-prefix=CHECK-EXEC %s
//CHECK-NOT: {{.*error|warning|note:.*}}

#include "clad/Differentiator/Differentiator.h"

double fn1(double i, double j) {
  double res = 0;
  int count = 1;
  switch (count) {
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
// CHECK-NEXT:     int count = 1;
// CHECK-NEXT:     {
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

double fn2(double i, double j) {
  double res = 0;
  switch (int count = 2) {
    res += i * i * j * j;
    res += 50 * i;
    case 0: res += i; break;
    case 1: res += j;
    case 2: res += i * j; break;
    default: res += i + j;
  }
  return res;
}

// CHECK: void fn2_grad(double i, double j, clad::array_ref<double> _d_i, clad::array_ref<double> _d_j) {
// CHECK-NEXT:     double _d_res = 0;
// CHECK-NEXT:     int _d_count = 0;
// CHECK-NEXT:     int _cond0;
// CHECK-NEXT:     double _t0;
// CHECK-NEXT:     double _t1;
// CHECK-NEXT:     double _t2;
// CHECK-NEXT:     double _t3;
// CHECK-NEXT:     double _t4;
// CHECK-NEXT:     double _t5;
// CHECK-NEXT:     double _t6;
// CHECK-NEXT:     clad::tape<unsigned long> _t7 = {};
// CHECK-NEXT:     double _t8;
// CHECK-NEXT:     double _t9;
// CHECK-NEXT:     double res = 0;
// CHECK-NEXT:     {
// CHECK-NEXT:         int count = 2;
// CHECK-NEXT:         _cond0 = count;
// CHECK-NEXT:         switch (_cond0) {
// CHECK-NEXT:             _t3 = i;
// CHECK-NEXT:             _t2 = i;
// CHECK-NEXT:             _t4 = _t3 * _t2;
// CHECK-NEXT:             _t1 = j;
// CHECK-NEXT:             _t5 = _t4 * _t1;
// CHECK-NEXT:             _t0 = j;
// CHECK-NEXT:             res += _t5 * _t0;
// CHECK-NEXT:             _t6 = i;
// CHECK-NEXT:             res += 50 * _t6;
// CHECK-NEXT:             {
// CHECK-NEXT:               case 0:
// CHECK-NEXT:                 res += i;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 clad::push(_t7, 1UL);
// CHECK-NEXT:                 break;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:               case 1:
// CHECK-NEXT:                 res += j;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:               case 2:
// CHECK-NEXT:                 res += _t9 * _t8;
// CHECK-NEXT:                 _t9 = i;
// CHECK-NEXT:                 _t8 = j;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 clad::push(_t7, 2UL);
// CHECK-NEXT:                 break;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:               default:
// CHECK-NEXT:                 res += i + j;
// CHECK-NEXT:             }
// CHECK-NEXT:             clad::push(_t7, 3UL);
// CHECK-NEXT:         }
// CHECK-NEXT:     }
// CHECK-NEXT:     double fn2_return = res;
// CHECK-NEXT:     goto _label0;
// CHECK-NEXT:   _label0:
// CHECK-NEXT:     _d_res += 1;
// CHECK-NEXT:     {
// CHECK-NEXT:         switch (clad::pop(_t7)) {
// CHECK-NEXT:           case 3UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     double _r_d5 = _d_res;
// CHECK-NEXT:                     _d_res += _r_d5;
// CHECK-NEXT:                     * _d_i += _r_d5;
// CHECK-NEXT:                     * _d_j += _r_d5;
// CHECK-NEXT:                     _d_res -= _r_d5;
// CHECK-NEXT:                 }
// CHECK-NEXT:                 if (_cond0 != 0 && _cond0 != 1 && _cond0 != 2)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:           case 2UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     double _r_d4 = _d_res;
// CHECK-NEXT:                     _d_res += _r_d4;
// CHECK-NEXT:                     double _r8 = _r_d4 * _t8;
// CHECK-NEXT:                     * _d_i += _r8;
// CHECK-NEXT:                     double _r9 = _t9 * _r_d4;
// CHECK-NEXT:                     * _d_j += _r9;
// CHECK-NEXT:                     _d_res -= _r_d4;
// CHECK-NEXT:                 }
// CHECK-NEXT:                 if (2 == _cond0)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     double _r_d3 = _d_res;
// CHECK-NEXT:                     _d_res += _r_d3;
// CHECK-NEXT:                     * _d_j += _r_d3;
// CHECK-NEXT:                     _d_res -= _r_d3;
// CHECK-NEXT:                 }
// CHECK-NEXT:                 if (1 == _cond0)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:           case 1UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     double _r_d2 = _d_res;
// CHECK-NEXT:                     _d_res += _r_d2;
// CHECK-NEXT:                     * _d_i += _r_d2;
// CHECK-NEXT:                     _d_res -= _r_d2;
// CHECK-NEXT:                 }
// CHECK-NEXT:                 if (0 == _cond0)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 double _r_d1 = _d_res;
// CHECK-NEXT:                 _d_res += _r_d1;
// CHECK-NEXT:                 double _r6 = _r_d1 * _t6;
// CHECK-NEXT:                 double _r7 = 50 * _r_d1;
// CHECK-NEXT:                 * _d_i += _r7;
// CHECK-NEXT:                 _d_res -= _r_d1;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 double _r_d0 = _d_res;
// CHECK-NEXT:                 _d_res += _r_d0;
// CHECK-NEXT:                 double _r0 = _r_d0 * _t0;
// CHECK-NEXT:                 double _r1 = _r0 * _t1;
// CHECK-NEXT:                 double _r2 = _r1 * _t2;
// CHECK-NEXT:                 * _d_i += _r2;
// CHECK-NEXT:                 double _r3 = _t3 * _r1;
// CHECK-NEXT:                 * _d_i += _r3;
// CHECK-NEXT:                 double _r4 = _t4 * _r0;
// CHECK-NEXT:                 * _d_j += _r4;
// CHECK-NEXT:                 double _r5 = _t5 * _r_d0;
// CHECK-NEXT:                 * _d_j += _r5;
// CHECK-NEXT:                 _d_res -= _r_d0;
// CHECK-NEXT:             }
// CHECK-NEXT:         }
// CHECK-NEXT:     }
// CHECK-NEXT: }

double fn3(double i, double j) {
  double res = 0;
  int counter = 2;
  while (counter--) {
    switch (counter) {
      case 0: res += i * i * j * j;
      case 1: {
        res += i * i;
      } break;
      case 2: res += j * j;
      default: res += i + j;
    }
  }
  return res;
}

// CHECK: void fn3_grad(double i, double j, clad::array_ref<double> _d_i, clad::array_ref<double> _d_j) {
// CHECK-NEXT:     double _d_res = 0;
// CHECK-NEXT:     int _d_counter = 0;
// CHECK-NEXT:     unsigned long _t0;
// CHECK-NEXT:     clad::tape<int> _cond0 = {};
// CHECK-NEXT:     clad::tape<double> _t1 = {};
// CHECK-NEXT:     clad::tape<double> _t2 = {};
// CHECK-NEXT:     clad::tape<double> _t3 = {};
// CHECK-NEXT:     clad::tape<double> _t4 = {};
// CHECK-NEXT:     clad::tape<double> _t5 = {};
// CHECK-NEXT:     clad::tape<double> _t6 = {};
// CHECK-NEXT:     clad::tape<double> _t7 = {};
// CHECK-NEXT:     clad::tape<double> _t8 = {};
// CHECK-NEXT:     clad::tape<unsigned long> _t9 = {};
// CHECK-NEXT:     clad::tape<double> _t10 = {};
// CHECK-NEXT:     clad::tape<double> _t11 = {};
// CHECK-NEXT:     double res = 0;
// CHECK-NEXT:     int counter = 2;
// CHECK-NEXT:     _t0 = 0;
// CHECK-NEXT:     while (counter--)
// CHECK-NEXT:         {
// CHECK-NEXT:             _t0++;
// CHECK-NEXT:             {
// CHECK-NEXT:                 switch (clad::push(_cond0, counter)) {
// CHECK-NEXT:                     {
// CHECK-NEXT:                       case 0:
// CHECK-NEXT:                         res += clad::push(_t6, clad::push(_t5, clad::push(_t4, i) * clad::push(_t3, i)) * clad::push(_t2, j)) * clad::push(_t1, j);
// CHECK-NEXT:                     }
// CHECK-NEXT:                     {
// CHECK-NEXT:                       case 1:
// CHECK-NEXT:                         {
// CHECK-NEXT:                             res += clad::push(_t8, i) * clad::push(_t7, i);
// CHECK-NEXT:                         }
// CHECK-NEXT:                     }
// CHECK-NEXT:                     {
// CHECK-NEXT:                         clad::push(_t9, 1UL);
// CHECK-NEXT:                         break;
// CHECK-NEXT:                     }
// CHECK-NEXT:                     {
// CHECK-NEXT:                       case 2:
// CHECK-NEXT:                         res += clad::push(_t11, j) * clad::push(_t10, j);
// CHECK-NEXT:                     }
// CHECK-NEXT:                     {
// CHECK-NEXT:                       default:
// CHECK-NEXT:                         res += i + j;
// CHECK-NEXT:                     }
// CHECK-NEXT:                     clad::push(_t9, 2UL);
// CHECK-NEXT:                 }
// CHECK-NEXT:             }
// CHECK-NEXT:         }
// CHECK-NEXT:     double fn3_return = res;
// CHECK-NEXT:     goto _label0;
// CHECK-NEXT:   _label0:
// CHECK-NEXT:     _d_res += 1;
// CHECK-NEXT:     while (_t0)
// CHECK-NEXT:         {
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     switch (clad::pop(_t9)) {
// CHECK-NEXT:                       case 2UL:
// CHECK-NEXT:                         ;
// CHECK-NEXT:                         {
// CHECK-NEXT:                             {
// CHECK-NEXT:                                 double _r_d3 = _d_res;
// CHECK-NEXT:                                 _d_res += _r_d3;
// CHECK-NEXT:                                 * _d_i += _r_d3;
// CHECK-NEXT:                                 * _d_j += _r_d3;
// CHECK-NEXT:                                 _d_res -= _r_d3;
// CHECK-NEXT:                             }
// CHECK-NEXT:                             if (clad::back(_cond0) != 0 && clad::back(_cond0) != 1 && clad::back(_cond0) != 2)
// CHECK-NEXT:                                 break;
// CHECK-NEXT:                         }
// CHECK-NEXT:                         {
// CHECK-NEXT:                             {
// CHECK-NEXT:                                 double _r_d2 = _d_res;
// CHECK-NEXT:                                 _d_res += _r_d2;
// CHECK-NEXT:                                 double _r8 = _r_d2 * clad::pop(_t10);
// CHECK-NEXT:                                 * _d_j += _r8;
// CHECK-NEXT:                                 double _r9 = clad::pop(_t11) * _r_d2;
// CHECK-NEXT:                                 * _d_j += _r9;
// CHECK-NEXT:                                 _d_res -= _r_d2;
// CHECK-NEXT:                             }
// CHECK-NEXT:                             if (2 == clad::back(_cond0))
// CHECK-NEXT:                                 break;
// CHECK-NEXT:                         }
// CHECK-NEXT:                       case 1UL:
// CHECK-NEXT:                         ;
// CHECK-NEXT:                         {
// CHECK-NEXT:                             {
// CHECK-NEXT:                                 {
// CHECK-NEXT:                                     double _r_d1 = _d_res;
// CHECK-NEXT:                                     _d_res += _r_d1;
// CHECK-NEXT:                                     double _r6 = _r_d1 * clad::pop(_t7);
// CHECK-NEXT:                                     * _d_i += _r6;
// CHECK-NEXT:                                     double _r7 = clad::pop(_t8) * _r_d1;
// CHECK-NEXT:                                     * _d_i += _r7;
// CHECK-NEXT:                                     _d_res -= _r_d1;
// CHECK-NEXT:                                 }
// CHECK-NEXT:                             }
// CHECK-NEXT:                             if (1 == clad::back(_cond0))
// CHECK-NEXT:                                 break;
// CHECK-NEXT:                         }
// CHECK-NEXT:                         {
// CHECK-NEXT:                             {
// CHECK-NEXT:                                 double _r_d0 = _d_res;
// CHECK-NEXT:                                 _d_res += _r_d0;
// CHECK-NEXT:                                 double _r0 = _r_d0 * clad::pop(_t1);
// CHECK-NEXT:                                 double _r1 = _r0 * clad::pop(_t2);
// CHECK-NEXT:                                 double _r2 = _r1 * clad::pop(_t3);
// CHECK-NEXT:                                 * _d_i += _r2;
// CHECK-NEXT:                                 double _r3 = clad::pop(_t4) * _r1;
// CHECK-NEXT:                                 * _d_i += _r3;
// CHECK-NEXT:                                 double _r4 = clad::pop(_t5) * _r0;
// CHECK-NEXT:                                 * _d_j += _r4;
// CHECK-NEXT:                                 double _r5 = clad::pop(_t6) * _r_d0;
// CHECK-NEXT:                                 * _d_j += _r5;
// CHECK-NEXT:                                 _d_res -= _r_d0;
// CHECK-NEXT:                             }
// CHECK-NEXT:                             if (0 == clad::back(_cond0))
// CHECK-NEXT:                                 break;
// CHECK-NEXT:                         }
// CHECK-NEXT:                     }
// CHECK-NEXT:                     clad::pop(_cond0);
// CHECK-NEXT:                 }
// CHECK-NEXT:             }
// CHECK-NEXT:             _t0--;
// CHECK-NEXT:         }
// CHECK-NEXT: }

double fn4(double i, double j) {
  double res = 0;
  switch (1) {
    case 0: res += i * i * j * j; break;
    case 1:
      int counter = 2;
      while (counter--) {
        res += i * j;
      }
      break;
  }
  return res;
}

// CHECK: void fn4_grad(double i, double j, clad::array_ref<double> _d_i, clad::array_ref<double> _d_j) {
// CHECK-NEXT:     double _d_res = 0;
// CHECK-NEXT:     double _t0;
// CHECK-NEXT:     double _t1;
// CHECK-NEXT:     double _t2;
// CHECK-NEXT:     double _t3;
// CHECK-NEXT:     double _t4;
// CHECK-NEXT:     double _t5;
// CHECK-NEXT:     clad::tape<unsigned long> _t6 = {};
// CHECK-NEXT:     int _d_counter = 0;
// CHECK-NEXT:     unsigned long _t7;
// CHECK-NEXT:     clad::tape<double> _t8 = {};
// CHECK-NEXT:     clad::tape<double> _t9 = {};
// CHECK-NEXT:     double res = 0;
// CHECK-NEXT:     {
// CHECK-NEXT:         switch (1) {
// CHECK-NEXT:             {
// CHECK-NEXT:               case 0:
// CHECK-NEXT:                 res += _t5 * _t0;
// CHECK-NEXT:                 _t3 = i;
// CHECK-NEXT:                 _t2 = i;
// CHECK-NEXT:                 _t4 = _t3 * _t2;
// CHECK-NEXT:                 _t1 = j;
// CHECK-NEXT:                 _t5 = _t4 * _t1;
// CHECK-NEXT:                 _t0 = j;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:                 clad::push(_t6, 1UL);
// CHECK-NEXT:                 break;
// CHECK-NEXT:             }
// CHECK-NEXT:             {
// CHECK-NEXT:               case 1:
// CHECK-NEXT:                 int counter = 2;
// CHECK-NEXT:             }
// CHECK-NEXT:             _t7 = 0;
// CHECK-NEXT:             while (counter--)
// CHECK-NEXT:                 {
// CHECK-NEXT:                     _t7++;
// CHECK-NEXT:                     res += clad::push(_t9, i) * clad::push(_t8, j);
// CHECK-NEXT:                 }
// CHECK-NEXT:             {
// CHECK-NEXT:                 clad::push(_t6, 2UL);
// CHECK-NEXT:                 break;
// CHECK-NEXT:             }
// CHECK-NEXT:             clad::push(_t6, 3UL);
// CHECK-NEXT:         }
// CHECK-NEXT:     }
// CHECK-NEXT:     double fn4_return = res;
// CHECK-NEXT:     goto _label0;
// CHECK-NEXT:   _label0:
// CHECK-NEXT:     _d_res += 1;
// CHECK-NEXT:     {
// CHECK-NEXT:         switch (clad::pop(_t6)) {
// CHECK-NEXT:           case 3UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:           case 2UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:             while (_t7)
// CHECK-NEXT:                 {
// CHECK-NEXT:                     {
// CHECK-NEXT:                         {
// CHECK-NEXT:                             double _r_d1 = _d_res;
// CHECK-NEXT:                             _d_res += _r_d1;
// CHECK-NEXT:                             double _r6 = _r_d1 * clad::pop(_t8);
// CHECK-NEXT:                             * _d_i += _r6;
// CHECK-NEXT:                             double _r7 = clad::pop(_t9) * _r_d1;
// CHECK-NEXT:                             * _d_j += _r7;
// CHECK-NEXT:                             _d_res -= _r_d1;
// CHECK-NEXT:                         }
// CHECK-NEXT:                     }
// CHECK-NEXT:                     _t7--;
// CHECK-NEXT:                 }
// CHECK-NEXT:             {
// CHECK-NEXT:                 if (1 == 1)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:           case 1UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:             {
// CHECK-NEXT:                 {
// CHECK-NEXT:                     double _r_d0 = _d_res;
// CHECK-NEXT:                     _d_res += _r_d0;
// CHECK-NEXT:                     double _r0 = _r_d0 * _t0;
// CHECK-NEXT:                     double _r1 = _r0 * _t1;
// CHECK-NEXT:                     double _r2 = _r1 * _t2;
// CHECK-NEXT:                     * _d_i += _r2;
// CHECK-NEXT:                     double _r3 = _t3 * _r1;
// CHECK-NEXT:                     * _d_i += _r3;
// CHECK-NEXT:                     double _r4 = _t4 * _r0;
// CHECK-NEXT:                     * _d_j += _r4;
// CHECK-NEXT:                     double _r5 = _t5 * _r_d0;
// CHECK-NEXT:                     * _d_j += _r5;
// CHECK-NEXT:                     _d_res -= _r_d0;
// CHECK-NEXT:                 }
// CHECK-NEXT:                 if (0 == 1)
// CHECK-NEXT:                     break;
// CHECK-NEXT:             }
// CHECK-NEXT:         }
// CHECK-NEXT:     }
// CHECK-NEXT: }

double fn5(double i, double j) {
  double res=0;
  switch(int count = 1)
    case 1:
      res += i*j;
  return res;
}

// CHECK: void fn5_grad(double i, double j, clad::array_ref<double> _d_i, clad::array_ref<double> _d_j) {
// CHECK-NEXT:     double _d_res = 0;
// CHECK-NEXT:     int _d_count = 0;
// CHECK-NEXT:     int _cond0;
// CHECK-NEXT:     double _t0;
// CHECK-NEXT:     double _t1;
// CHECK-NEXT:     clad::tape<unsigned long> _t2 = {};
// CHECK-NEXT:     double res = 0;
// CHECK-NEXT:     {
// CHECK-NEXT:         int count = 1;
// CHECK-NEXT:         _cond0 = count;
// CHECK-NEXT:         switch (_cond0) {
// CHECK-NEXT:           case 1:
// CHECK-NEXT:             res += _t1 * _t0;
// CHECK-NEXT:             _t1 = i;
// CHECK-NEXT:             _t0 = j;
// CHECK-NEXT:             clad::push(_t2, 1UL);
// CHECK-NEXT:         }
// CHECK-NEXT:     }
// CHECK-NEXT:     double fn5_return = res;
// CHECK-NEXT:     goto _label0;
// CHECK-NEXT:   _label0:
// CHECK-NEXT:     _d_res += 1;
// CHECK-NEXT:     {
// CHECK-NEXT:         switch (clad::pop(_t2)) {
// CHECK-NEXT:           case 1UL:
// CHECK-NEXT:             ;
// CHECK-NEXT:             {
// CHECK-NEXT:                 double _r_d0 = _d_res;
// CHECK-NEXT:                 _d_res += _r_d0;
// CHECK-NEXT:                 double _r0 = _r_d0 * _t0;
// CHECK-NEXT:                 * _d_i += _r0;
// CHECK-NEXT:                 double _r1 = _t1 * _r_d0;
// CHECK-NEXT:                 * _d_j += _r1;
// CHECK-NEXT:                 _d_res -= _r_d0;
// CHECK-NEXT:             }
// CHECK-NEXT:             if (1 == _cond0)
// CHECK-NEXT:                 break;
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

  TEST_2(fn1, 3, 5);  // CHECK-EXEC: {156.00, 100.00}
  TEST_2(fn2, 3, 5);  // CHECK-EXEC: {5.00, 3.00}
  TEST_2(fn3, 3, 5);  // CHECK-EXEC: {162.00, 90.00}
  TEST_2(fn4, 3, 5);  // CHECK-EXEC: {10.00, 6.00}
  TEST_2(fn5, 3, 5);  // CHECK-EXEC: {5.00, 3.00}
}