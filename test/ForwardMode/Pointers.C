// RUN: %cladclang %s -I%S/../../include -oPointers.out 2>&1 | FileCheck %s
// RUN: ./Pointers.out | FileCheck -check-prefix=CHECK-EXEC %s
// CHECK-NOT: {{.*error|warning|note:.*}}

#include "clad/Differentiator/Differentiator.h"

using namespace std;

class Expression {
public:
  Expression(double p_x=0,double p_y=0) : x(p_x), y(p_y) {}
  double x,y;
  double memFn(double i,double j) {
    return i*i*j*x + j*j*y;
  }

  // CHECK: double memFn_darg0(double i, double j) {
  // CHECK-NEXT:     double _d_i = 1;
  // CHECK-NEXT:     double _d_j = 0;
  // CHECK-NEXT:     double _t0 = i * i;
  // CHECK-NEXT:     double _t1 = _t0 * j;
  // CHECK-NEXT:     double &_t2 = this->x;
  // CHECK-NEXT:     double _t3 = j * j;
  // CHECK-NEXT:     double &_t4 = this->y;
  // CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * _t2 + _t1 * 0. + (_d_j * j + j * _d_j) * _t4 + _t3 * 0.;
  // CHECK-NEXT: }
  
  // CHECK: double memFn_darg0(double i, double j) {
  // CHECK-NEXT:     double _d_i = 1;
  // CHECK-NEXT:     double _d_j = 0;
  // CHECK-NEXT:     double _t0 = i * i;
  // CHECK-NEXT:     double _t1 = _t0 * j;
  // CHECK-NEXT:     double &_t2 = this->x;
  // CHECK-NEXT:     double _t3 = j * j;
  // CHECK-NEXT:     double &_t4 = this->y;
  // CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * _t2 + _t1 * 0. + (_d_j * j + j * _d_j) * _t4 + _t3 * 0.;
  // CHECK-NEXT: }
  
  // CHECK: double memFn_darg0(double i, double j) {
  // CHECK-NEXT:     double _d_i = 1;
  // CHECK-NEXT:     double _d_j = 0;
  // CHECK-NEXT:     double _t0 = i * i;
  // CHECK-NEXT:     double _t1 = _t0 * j;
  // CHECK-NEXT:     double &_t2 = this->x;
  // CHECK-NEXT:     double _t3 = j * j;
  // CHECK-NEXT:     double &_t4 = this->y;
  // CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * _t2 + _t1 * 0. + (_d_j * j + j * _d_j) * _t4 + _t3 * 0.;
  // CHECK-NEXT: }
  
  // CHECK: double memFn_darg0(double i, double j) {
  // CHECK-NEXT:     double _d_i = 1;
  // CHECK-NEXT:     double _d_j = 0;
  // CHECK-NEXT:     double _t0 = i * i;
  // CHECK-NEXT:     double _t1 = _t0 * j;
  // CHECK-NEXT:     double &_t2 = this->x;
  // CHECK-NEXT:     double _t3 = j * j;
  // CHECK-NEXT:     double &_t4 = this->y;
  // CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * _t2 + _t1 * 0. + (_d_j * j + j * _d_j) * _t4 + _t3 * 0.;
  // CHECK-NEXT: }
  
  // CHECK: double memFn_darg0(double i, double j) {
  // CHECK-NEXT:     double _d_i = 1;
  // CHECK-NEXT:     double _d_j = 0;
  // CHECK-NEXT:     double _t0 = i * i;
  // CHECK-NEXT:     double _t1 = _t0 * j;
  // CHECK-NEXT:     double &_t2 = this->x;
  // CHECK-NEXT:     double _t3 = j * j;
  // CHECK-NEXT:     double &_t4 = this->y;
  // CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * _t2 + _t1 * 0. + (_d_j * j + j * _d_j) * _t4 + _t3 * 0.;
  // CHECK-NEXT: }
  
  // CHECK: double memFn_darg0(double i, double j) {
  // CHECK-NEXT:     double _d_i = 1;
  // CHECK-NEXT:     double _d_j = 0;
  // CHECK-NEXT:     double _t0 = i * i;
  // CHECK-NEXT:     double _t1 = _t0 * j;
  // CHECK-NEXT:     double &_t2 = this->x;
  // CHECK-NEXT:     double _t3 = j * j;
  // CHECK-NEXT:     double &_t4 = this->y;
  // CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * _t2 + _t1 * 0. + (_d_j * j + j * _d_j) * _t4 + _t3 * 0.;
  // CHECK-NEXT: }
  
  // CHECK: double memFn_darg0(double i, double j) {
  // CHECK-NEXT:     double _d_i = 1;
  // CHECK-NEXT:     double _d_j = 0;
  // CHECK-NEXT:     double _t0 = i * i;
  // CHECK-NEXT:     double _t1 = _t0 * j;
  // CHECK-NEXT:     double &_t2 = this->x;
  // CHECK-NEXT:     double _t3 = j * j;
  // CHECK-NEXT:     double &_t4 = this->y;
  // CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * _t2 + _t1 * 0. + (_d_j * j + j * _d_j) * _t4 + _t3 * 0.;
  // CHECK-NEXT: }

};

double nonMemFn(double i,double j) {
  return i*i*j*j;
}

// CHECK: double nonMemFn_darg0(double i, double j) {
// CHECK-NEXT:     double _d_i = 1;
// CHECK-NEXT:     double _d_j = 0;
// CHECK-NEXT:     double _t0 = i * i;
// CHECK-NEXT:     double _t1 = _t0 * j;
// CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * j + _t1 * _d_j;
// CHECK-NEXT: }

// CHECK: double nonMemFn_darg0(double i, double j) {
// CHECK-NEXT:     double _d_i = 1;
// CHECK-NEXT:     double _d_j = 0;
// CHECK-NEXT:     double _t0 = i * i;
// CHECK-NEXT:     double _t1 = _t0 * j;
// CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * j + _t1 * _d_j;
// CHECK-NEXT: }

// CHECK: double nonMemFn_darg0(double i, double j) {
// CHECK-NEXT:     double _d_i = 1;
// CHECK-NEXT:     double _d_j = 0;
// CHECK-NEXT:     double _t0 = i * i;
// CHECK-NEXT:     double _t1 = _t0 * j;
// CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * j + _t1 * _d_j;
// CHECK-NEXT: }

// CHECK: double nonMemFn_darg0(double i, double j) {
// CHECK-NEXT:     double _d_i = 1;
// CHECK-NEXT:     double _d_j = 0;
// CHECK-NEXT:     double _t0 = i * i;
// CHECK-NEXT:     double _t1 = _t0 * j;
// CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * j + _t1 * _d_j;
// CHECK-NEXT: }

// CHECK: double nonMemFn_darg0(double i, double j) {
// CHECK-NEXT:     double _d_i = 1;
// CHECK-NEXT:     double _d_j = 0;
// CHECK-NEXT:     double _t0 = i * i;
// CHECK-NEXT:     double _t1 = _t0 * j;
// CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * j + _t1 * _d_j;
// CHECK-NEXT: }

// CHECK: double nonMemFn_darg0(double i, double j) {
// CHECK-NEXT:     double _d_i = 1;
// CHECK-NEXT:     double _d_j = 0;
// CHECK-NEXT:     double _t0 = i * i;
// CHECK-NEXT:     double _t1 = _t0 * j;
// CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * j + _t1 * _d_j;
// CHECK-NEXT: }

// CHECK: double nonMemFn_darg0(double i, double j) {
// CHECK-NEXT:     double _d_i = 1;
// CHECK-NEXT:     double _d_j = 0;
// CHECK-NEXT:     double _t0 = i * i;
// CHECK-NEXT:     double _t1 = _t0 * j;
// CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * j + _t1 * _d_j;
// CHECK-NEXT: }

// CHECK: double nonMemFn_darg0(double i, double j) {
// CHECK-NEXT:     double _d_i = 1;
// CHECK-NEXT:     double _d_j = 0;
// CHECK-NEXT:     double _t0 = i * i;
// CHECK-NEXT:     double _t1 = _t0 * j;
// CHECK-NEXT:     return ((_d_i * i + i * _d_i) * j + _t0 * _d_j) * j + _t1 * _d_j;
// CHECK-NEXT: }

#define MEM_FN_TEST(var)\
printf("%.2f\n",var.execute(expr,7,9));\
printf("%.2f\n",var.execute(expr,11,9));

#define NON_MEM_FN_TEST(var)\
printf("%.2f\n",var.execute(3,4));\
printf("%.2f\n",var.execute(7,5));


int main() {
  Expression expr(5,7);

  auto memFnPtr = &Expression::memFn;
  auto memFnPtrToPtr = &memFnPtr;
  auto memFnPtrToPtrToPtr = &memFnPtrToPtr;
  auto memFnIndirectPtr = memFnPtr;
  auto memFnIndirectIndirectPtr = memFnIndirectPtr;

  auto d_memFnPtr = clad::differentiate(memFnPtr,"i");
  auto d_memFnPtrToPtr = clad::differentiate(*memFnPtrToPtr,"i");
  auto d_memFnPtrToPtr_1 = clad::differentiate(**&memFnPtrToPtr,"i");
  auto d_memFnPtrToPtrToPtr = clad::differentiate(**memFnPtrToPtrToPtr,"i");
  auto d_memFnPtrToPtrToPtr_1 = clad::differentiate(***&memFnPtrToPtrToPtr,"i");
  auto d_memFnIndirectPtr = clad::differentiate(memFnIndirectPtr,"i");
  auto d_memFnIndirectIndirectPtr = clad::differentiate(memFnIndirectIndirectPtr,"i");

  MEM_FN_TEST(d_memFnPtr);  // CHECK-EXEC: 630.00
                            // CHECK-EXEC: 990.00

  MEM_FN_TEST(d_memFnPtrToPtr); // CHECK-EXEC: 630.00
                                // CHECK-EXEC: 990.00

  MEM_FN_TEST(d_memFnPtrToPtr_1); // CHECK-EXEC: 630.00
                                  // CHECK-EXEC: 990.00

  MEM_FN_TEST(d_memFnPtrToPtrToPtr);  // CHECK-EXEC: 630.00
                                      // CHECK-EXEC: 990.00

  MEM_FN_TEST(d_memFnPtrToPtrToPtr_1);  // CHECK-EXEC: 630.00
                                        // CHECK-EXEC: 990.00

  MEM_FN_TEST(d_memFnIndirectPtr);  // CHECK-EXEC: 630.00
                                    // CHECK-EXEC: 990.00

  MEM_FN_TEST(d_memFnIndirectIndirectPtr);  // CHECK-EXEC: 630.00
                                            // CHECK-EXEC: 990.00

  auto nonMemFnPtr = &nonMemFn;
  auto nonMemFnPtrToPtr = &nonMemFnPtr;
  auto nonMemFnPtrToPtrToPtr = &nonMemFnPtrToPtr;
  auto nonMemFnIndirectPtr = nonMemFnPtr;
  auto nonMemFnIndirectIndirectPtr = nonMemFnIndirectPtr;

  auto d_nonMemFn = clad::differentiate(nonMemFn,"i");
  auto d_nonMemFnPtr = clad::differentiate(nonMemFnPtr,"i");
  auto d_nonMemFnPtrToPtr = clad::differentiate(*nonMemFnPtrToPtr,"i");
  auto d_nonMemFnPtrToPtr_1 = clad::differentiate(**&nonMemFnPtrToPtr,"i");
  auto d_nonMemFnPtrToPtrToPtr = clad::differentiate(**nonMemFnPtrToPtrToPtr,"i");
  auto d_nonMemFnPtrToPtrToPtr_1 = clad::differentiate(***&nonMemFnPtrToPtrToPtr,"i");
  auto d_nonMemFnIndirectPtr = clad::differentiate(nonMemFnIndirectPtr,"i");
  auto d_nonMemFnIndirectIndirectPtr = clad::differentiate(nonMemFnIndirectIndirectPtr,"i");

  NON_MEM_FN_TEST(d_nonMemFn);  // CHECK-EXEC: 96.00
                                // CHECK-EXEC: 350.00

  NON_MEM_FN_TEST(d_nonMemFnPtr); // CHECK-EXEC: 96.00
                                  // CHECK-EXEC: 350.00

  NON_MEM_FN_TEST(d_nonMemFnPtrToPtr);  // CHECK-EXEC: 96.00
                                        // CHECK-EXEC: 350.00

  NON_MEM_FN_TEST(d_nonMemFnPtrToPtr_1);  // CHECK-EXEC: 96.00
                                          // CHECK-EXEC: 350.00

  NON_MEM_FN_TEST(d_nonMemFnPtrToPtrToPtr); // CHECK-EXEC: 96.00
                                            // CHECK-EXEC: 350.00

  NON_MEM_FN_TEST(d_nonMemFnPtrToPtrToPtr_1); // CHECK-EXEC: 96.00
                                              // CHECK-EXEC: 350.00

  NON_MEM_FN_TEST(d_nonMemFnIndirectPtr); // CHECK-EXEC: 96.00
                                          // CHECK-EXEC: 350.00

  NON_MEM_FN_TEST(d_nonMemFnIndirectIndirectPtr); // CHECK-EXEC: 96.00
                                                  // CHECK-EXEC: 350.00

}