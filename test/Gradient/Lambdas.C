// RUN: %cladclang -Xclang -plugin-arg-clad -Xclang -disable-tbr %s -I%S/../../include -oLambdas.out 2>&1 | %filecheck %s
// RUN: ./Lambdas.out | %filecheck_exec %s
// RUN: %cladclang %s -I%S/../../include -oLambdas.out
// RUN: ./Lambdas.out | %filecheck_exec %s

#include "clad/Differentiator/Differentiator.h"
#include <cstdio> // For printf

double f1(double i, double j) {
  auto _f = [] (double t) {
    return t*t + 1.0;
  };
  return i + _f(j);
}

// CHECK:     define internal void @_ZNK3$_0clEdd(ptr noundef nonnull align 8 dereferenceable(8) %this, double %t, double %_d_y, ptr noundef %_d_t)
// CHECK-SAME: #[[LAMBDA_PULLBACK_ATTRS:[0-9]+]] {
// CHECK-NEXT: entry:
// CHECK-NEXT:   %[[D_Y_ADDR:.+]] = alloca double, align 8
// CHECK-NEXT:   %[[D_T_ADDR:.+]] = alloca ptr, align 8
// CHECK-NEXT:   store double %_d_y, ptr %[[D_Y_ADDR]], align 8
// CHECK-NEXT:   store ptr %_d_t, ptr %[[D_T_ADDR]], align 8
// CHECK-NEXT:   %[[LOAD_D_Y:.+]] = load double, ptr %[[D_Y_ADDR]], align 8
// CHECK-NEXT:   %[[MUL1:.+]] = fmul double %[[LOAD_D_Y]], %t
// CHECK-NEXT:   %[[LOAD_D_T_PTR:.+]] = load ptr, ptr %[[D_T_ADDR]], align 8
// CHECK-NEXT:   %[[LOAD_D_T_VAL:.+]] = load double, ptr %[[LOAD_D_T_PTR]], align 8
// CHECK-NEXT:   %[[ADD1:.+]] = fadd double %[[LOAD_D_T_VAL]], %[[MUL1]]
// CHECK-NEXT:   store double %[[ADD1]], ptr %[[LOAD_D_T_PTR]], align 8
// CHECK-NEXT:   %[[MUL2:.+]] = fmul double %t, %[[LOAD_D_Y]]
// CHECK-NEXT:   %[[LOAD_D_T_PTR2:.+]] = load ptr, ptr %[[D_T_ADDR]], align 8
// CHECK-NEXT:   %[[LOAD_D_T_VAL2:.+]] = load double, ptr %[[LOAD_D_T_PTR2]], align 8
// CHECK-NEXT:   %[[ADD2:.+]] = fadd double %[[LOAD_D_T_VAL2]], %[[MUL2]]
// CHECK-NEXT:   store double %[[ADD2]], ptr %[[LOAD_D_T_PTR2]], align 8
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

// CHECK:     define {{.*}}void @f1_grad(double %i, double %j, ptr %_d_i, ptr %_d_j)
// CHECK-NEXT: entry:
// CHECK:         %[[LAMBDA_OBJ:.+]] = alloca %class.anon
// CHECK:         store double 1.000000e+00, ptr %_d_i, align 8
// CHECK:         %_r0 = alloca double, align 8
// CHECK-NEXT:    store double 0.000000e+00, ptr %_r0, align 8
// CHECK-NEXT:    call void @_ZNK3$_0clEdd(ptr {{[^,]*}} %[[LAMBDA_OBJ]], double %j, double 1.000000e+00, ptr %_r0)
// CHECK-NEXT:    %[[LOAD_R0:.+]] = load double, ptr %_r0, align 8
// CHECK-NEXT:    %[[LOAD_D_J:.+]] = load double, ptr %_d_j, align 8
// CHECK-NEXT:    %[[ADD_DJ:.+]] = fadd double %[[LOAD_D_J]], %[[LOAD_R0]]
// CHECK-NEXT:    store double %[[ADD_DJ]], ptr %_d_j, align 8
// CHECK-NEXT:    ret void
// CHECK-NEXT:  }

double f2(double i, double j) {
  auto _f = [] (double t, double k) {
    return t + k;
  };
  double x = _f(i + j, i);
  return x;
}

// CHECK: define internal void @_ZNK3$_1clEddd(ptr noundef nonnull align 8 dereferenceable(8) %this, double %t, double %k, double %_d_y, ptr noundef %_d_t, ptr noundef %_d_k)
// CHECK:         {
// CHECK-NEXT:             *_d_t += _d_y;
// CHECK-NEXT:             *_d_k += _d_y;
// CHECK-NEXT:         }

// CHECK:    define {{.*}}void @f2_grad(double %i, double %j, ptr %_d_i, ptr %_d_j)
// CHECK:        double _d_x = 0.;
// CHECK-NEXT:        double x = _f(i + j, i);
// CHECK-NEXT:        _d_x += 1;
// CHECK-NEXT:        {
// CHECK-NEXT:            double _r0 = 0.;
// CHECK-NEXT:            double _r1 = 0.;
// CHECK-NEXT:            _f.operator_call_pullback(i + j, i, _d_x, &_r0, &_r1);
// CHECK-NEXT:            *_d_i += _r0;
// CHECK-NEXT:            *_d_j += _r0;
// CHECK-NEXT:            *_d_i += _r1;
// CHECK-NEXT:        }

// Test Case: Capture by Reference - Simple Usage
double f_ref_simple(double x, double y_ref_orig) {
  double& y_ref = y_ref_orig;
  auto lambda_ref_simple = [&](double val) {
    return val * y_ref;
  };
  return lambda_ref_simple(x) + y_ref;
}
// CHECK: define internal void @_ZZ12f_ref_simpleddENK3$_2clEdRd(ptr{{.+}}, double{{.+}}, ptr{{.+}}, ptr{{.+}}, ptr{{.+}})
// CHECK: entry:
// CHECK: %[[ADJOINT_Y_REF_ADDR:.+]] = load ptr, ptr %{{.+}}, align 8
// CHECK: %[[ADJOINT_VAL_ADDR:.+]] = load ptr, ptr %{{.+}}, align 8
// CHECK: %[[D_RET_VAL:.+]] = load double, ptr %{{.+}}, align 8
// CHECK: %[[Y_REF_VAL:.+]] = load double, ptr %{{.+}}, align 8
// CHECK: %[[MUL_DY_YREF:.+]] = fmul double %[[D_RET_VAL]], %[[Y_REF_VAL]]
// CHECK: %[[LOAD_ADJOINT_VAL:.+]] = load double, ptr %[[ADJOINT_VAL_ADDR]], align 8
// CHECK: %[[ADD_ADJOINT_VAL:.+]] = fadd double %[[LOAD_ADJOINT_VAL]], %[[MUL_DY_YREF]]
// CHECK: store double %[[ADD_ADJOINT_VAL]], ptr %[[ADJOINT_VAL_ADDR]], align 8
// CHECK: %[[VAL_VAL:.+]] = load double, ptr %{{.+}}, align 8
// CHECK: %[[MUL_DY_VAL:.+]] = fmul double %[[D_RET_VAL]], %[[VAL_VAL]]
// CHECK: %[[LOAD_ADJOINT_Y_REF:.+]] = load double, ptr %[[ADJOINT_Y_REF_ADDR]], align 8
// CHECK: %[[ADD_ADJOINT_Y_REF:.+]] = fadd double %[[LOAD_ADJOINT_Y_REF:.+]], %[[MUL_DY_VAL]]
// CHECK: store double %[[ADD_ADJOINT_Y_REF]], ptr %[[ADJOINT_Y_REF_ADDR]], align 8

// Test Case: Capture by Reference - Modification
double f_ref_modify(double x, double y_ref_orig) {
  double& y_ref = y_ref_orig;
  auto lambda_ref_modify = [&](double val) mutable {
    y_ref += val; // y_ref is modified
    return y_ref * 2;
  };
  return lambda_ref_modify(x);
}

// Test Case: Capture by Value - Simple Usage
double f_val_simple(double x, double y_val_orig_param) {
  auto lambda_val_simple = [y_val_orig_param](double val) { // y_val_orig_param is captured by value
    return val * y_val_orig_param;
  };
  return lambda_val_simple(x) + y_val_orig_param;
}
// CHECK: define internal void @_ZZ12f_val_simpleddENK3$_4clEdd(ptr{{.+}}, double{{.+}}, double{{.+}}, ptr{{.+}}, ptr{{.+}})
// CHECK: %[[FIELD_ADDR:.+]] = getelementptr inbounds %{{.+}}, ptr %{{.+}}, i32 0, i32 0
// CHECK: %[[ADJOINT_Y_VAL_ORIG_PARAM_ADDR:.+]] = load ptr, ptr %{{.+}}, align 8
// CHECK: %[[ADJOINT_VAL_ADDR:.+]] = load ptr, ptr %{{.+}}, align 8
// CHECK: %[[D_RET_VAL:.+]] = load double, ptr %{{.+}}, align 8
// CHECK: %[[Y_VAL_ORIG_COPY_VAL:.+]] = load double, ptr %[[FIELD_ADDR]], align 8
// CHECK: %[[MUL_DY_YVALCOPY:.+]] = fmul double %[[D_RET_VAL]], %[[Y_VAL_ORIG_COPY_VAL]]
// CHECK: %[[LOAD_ADJOINT_VAL:.+]] = load double, ptr %[[ADJOINT_VAL_ADDR]], align 8
// CHECK: %[[ADD_ADJOINT_VAL:.+]] = fadd double %[[LOAD_ADJOINT_VAL]], %[[MUL_DY_YVALCOPY]]
// CHECK: store double %[[ADD_ADJOINT_VAL]], ptr %[[ADJOINT_VAL_ADDR]], align 8
// CHECK: %[[VAL_VAL:.+]] = load double, ptr %{{.+}}, align 8
// CHECK: %[[MUL_DY_VAL:.+]] = fmul double %[[D_RET_VAL]], %[[VAL_VAL]]
// CHECK: %[[LOAD_ADJOINT_Y_VAL_ORIG_PARAM:.+]] = load double, ptr %[[ADJOINT_Y_VAL_ORIG_PARAM_ADDR]], align 8
// CHECK: %[[ADD_ADJOINT_Y_VAL_ORIG_PARAM:.+]] = fadd double %[[LOAD_ADJOINT_Y_VAL_ORIG_PARAM:.+]], %[[MUL_DY_VAL]]
// CHECK: store double %[[ADD_ADJOINT_Y_VAL_ORIG_PARAM]], ptr %[[ADJOINT_Y_VAL_ORIG_PARAM_ADDR]], align 8


// Test Case: Capture by Value - Modification (Mutable Lambda)
double f_val_modify(double x, double y_val_orig_param) {
  auto lambda_val_modify = [y_val_orig_param](double val) mutable {
    double old_y_val = y_val_orig_param;
    y_val_orig_param = y_val_orig_param + val;
    return y_val_orig_param * old_y_val;
  };
  return lambda_val_modify(x);
}

// Test Case: Multiple Captures (Mix)
double f_multiple_captures(double x, double y_ref_orig, double z_val_orig) {
  double& y_ref = y_ref_orig;
  auto lambda_multi = [&y_ref, z_val_orig](double val) mutable {
    y_ref += val;
    return y_ref * z_val_orig;
  };
  return lambda_multi(x);
}

// Test Case: Capture this - Implicit Member Access
struct MyClassThis {
  double member_var;
  MyClassThis(double mv) : member_var(mv) {}

  double compute(double val) {
    auto lambda_this_implicit = [&](double x_lambda_param) { // Captures this implicitly
      return x_lambda_param * member_var; // Accesses this->member_var
    };
    return lambda_this_implicit(val) + member_var;
  }
};


int main() {
  double di = 0, dj = 0, dx = 0, dy_ref_orig = 0, dz_val_orig = 0, dy_val_orig = 0;
  double d_val = 0, d_member_var = 0;

  auto df1 = clad::gradient(f1);
  df1.execute(3, 4, &di, &dj);
  printf("f1(3,4): d/di=%.2f, d/dj=%.2f\n", di, dj); // CHECK-EXEC: f1(3,4): d/di=1.00, d/dj=8.00

  di = 0; dj = 0;
  auto df2 = clad::gradient(f2);
  df2.execute(3, 4, &di, &dj);
  printf("f2(3,4): d/di=%.2f, d/dj=%.2f\n", di, dj); // CHECK-EXEC: f2(3,4): d/di=2.00, d/dj=1.00

  dx = 0; dy_ref_orig = 0;
  auto df_ref_simple = clad::gradient(f_ref_simple);
  df_ref_simple.execute(2.0, 3.0, &dx, &dy_ref_orig);
  // Expected: d/dx = y_ref_orig (3.0), d/dy_ref_orig = x + 1 (2.0 + 1 = 3.0)
  printf("f_ref_simple(2,3): d/dx=%.2f, d/dy_ref_orig=%.2f\n", dx, dy_ref_orig); // CHECK-EXEC: f_ref_simple(2,3): d/dx=3.00, d/dy_ref_orig=3.00

  dx = 0; dy_ref_orig = 0;
  auto df_ref_modify = clad::gradient(f_ref_modify);
  df_ref_modify.execute(2.0, 3.0, &dx, &dy_ref_orig);
  // y_ref becomes 3+2=5. Returns 5*2=10.
  // d/dx = (d(y_ref)/dx)*2 = 1*2 = 2.
  // d/dy_ref_orig = (d(y_ref)/dy_ref_orig)*2 = 1*2 = 2.
  printf("f_ref_modify(2,3): d/dx=%.2f, d/dy_ref_orig=%.2f\n", dx, dy_ref_orig); // CHECK-EXEC: f_ref_modify(2,3): d/dx=2.00, d/dy_ref_orig=2.00

  dx = 0; dy_val_orig = 0;
  auto df_val_simple = clad::gradient(f_val_simple);
  df_val_simple.execute(2.0, 3.0, &dx, &dy_val_orig);
  // Expected: d/dx = y_val_orig (3.0), d/dy_val_orig_param = x + 1 (2.0 + 1 = 3.0)
  printf("f_val_simple(2,3): d/dx=%.2f, d/dy_val_orig=%.2f\n", dx, dy_val_orig); // CHECK-EXEC: f_val_simple(2,3): d/dx=3.00, d/dy_val_orig=3.00

  dx = 0; dy_val_orig = 0;
  auto df_val_modify = clad::gradient(f_val_modify);
  df_val_modify.execute(2.0, 3.0, &dx, &dy_val_orig);
  // Lambda returns (y_val_orig_param + x) * y_val_orig_param = (3+2)*3 = 15
  // d/dx = y_val_orig_param (3.0)
  // d/dy_val_orig_param = 2*y_val_orig_param + x = 2*3 + 2 = 8.0
  printf("f_val_modify(2,3): d/dx=%.2f, d/dy_val_orig=%.2f\n", dx, dy_val_orig); // CHECK-EXEC: f_val_modify(2,3): d/dx=3.00, d/dy_val_orig=8.00

  dx = 0; dy_ref_orig = 0; dz_val_orig = 0;
  auto df_multiple_captures = clad::gradient(f_multiple_captures);
  df_multiple_captures.execute(2.0, 3.0, 4.0, &dx, &dy_ref_orig, &dz_val_orig);
  // y_ref becomes 3+2=5. Lambda returns 5*4=20.
  // d/dx = z_val_orig (4.0)
  // d/dy_ref_orig = z_val_orig (4.0)
  // d/dz_val_orig = y_ref_orig + x (3.0 + 2.0 = 5.0)
  printf("f_multiple_captures(2,3,4): d/dx=%.2f, d/dy_ref_orig=%.2f, d/dz_val_orig=%.2f\n", dx, dy_ref_orig, dz_val_orig); // CHECK-EXEC: f_multiple_captures(2,3,4): d/dx=4.00, d/dy_ref_orig=4.00, d/dz_val_orig=5.00

  MyClassThis obj_this(10.0);
  auto df_this_capture = clad::gradient(obj_this, &MyClassThis::compute, "val, member_var");
  d_val = 0; d_member_var = 0;
  df_this_capture.execute(5.0, &d_val, &d_member_var);
  // lambda returns 5 * 10 = 50. compute returns 50 + 10 = 60.
  // d/dval = member_var (10.0)
  // d/dmember_var = val + 1 (5.0 + 1 = 6.0)
  printf("MyClassThis::compute(5.0) [member_var=10.0]: d/dval=%.2f, d/dmember_var=%.2f\n", d_val, d_member_var); // CHECK-EXEC: MyClassThis::compute(5.0) [member_var=10.0]: d/dval=10.00, d/dmember_var=6.00

  return 0;
}

[end of test/Gradient/Lambdas.C]
