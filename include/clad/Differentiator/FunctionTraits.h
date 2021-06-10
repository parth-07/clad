#ifndef FUNCTION_TRAITS
#define FUNCTION_TRAITS

#include <type_traits>

namespace clad {

  // Trait class to deduce return type of function(both member and non-member) at commpile time
  // Only function pointer types are supported by this trait class
  template <class F> 
  struct return_type {};
  template <class F> 
  using return_type_t = typename return_type<F>::type;

  // specializations for non-member functions pointer types
  template <class ReturnType, class... Args> 
  struct return_type<ReturnType (*)(Args...)> {
    using type = ReturnType;
  };
  template <class ReturnType, class... Args> 
  struct return_type<ReturnType (*)(Args..., ...)> {
    using type = ReturnType;
  };

  // specializations for member functions pointer types with no qualifiers
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...)> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...)> { 
    using type = ReturnType; 
  };

  // specializations for member functions pointer type with only cv-qualifiers
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) volatile> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) volatile> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const volatile> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const volatile> { 
    using type = ReturnType; 
  };

  // specializations for member functions pointer types with 
  // reference qualifiers and with and without cv-qualifiers
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) &> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) &> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const &> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const &> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) volatile &> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) volatile &> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const volatile &> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const volatile &> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) &&> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) &&> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const &&> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const &&> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) volatile &&> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) volatile &&> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const volatile &&> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const volatile &&> { 
    using type = ReturnType; 
  };

  // specializations for noexcept member functions
  #if __cpp_noexcept_function_type > 0
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) volatile noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) volatile noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const volatile noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const volatile noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) & noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) & noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const & noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const & noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) volatile & noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) volatile & noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const volatile & noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const volatile & noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) && noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) && noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const && noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const && noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) volatile && noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) volatile && noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args...) const volatile && noexcept> { 
    using type = ReturnType; 
  };
  template <class ReturnType, class C, class... Args> 
  struct return_type<ReturnType (C::*)(Args..., ...) const volatile && noexcept> { 
    using type = ReturnType; 
  };
  #endif

  // ExtractDerivedFnTraits is used to deduce type of the derived functions 
  // derived using reverse, hessian and jacobian differentiation modes
  // It SHOULD NOT be used to get traits of derived functions derived using
  // forward differentiation mode
  template<class ReturnType>
  struct ExtractDerivedFnTraits {};
  template<class T>
  using ExtractDerivedFnTraits_t = typename ExtractDerivedFnTraits<T>::type;

  // specializations for non-member functions pointer types
  template <class ReturnType,class... Args>
  struct ExtractDerivedFnTraits<ReturnType (*)(Args...)> {
    using type = void (*)(Args..., ReturnType*);
  };

  // specializations for member functions pointer types with no cv-qualifiers
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...)> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  
  // specializations for member functions pointer types with only cv-qualifiers
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) volatile> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const volatile> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };

  // specializations for member functions pointer types with 
  // reference qualifiers and with and without cv-qualifiers
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) &> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const &> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) volatile &> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const volatile &> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) &&> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const &&> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) volatile &&> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const volatile &&> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };

  // specializations for noexcept member functions
  #if __cpp_noexcept_function_type > 0
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) volatile noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const volatile noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) & noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const & noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) volatile & noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const volatile & noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) && noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const && noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) volatile && noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  template <class ReturnType, class C, class... Args> 
  struct ExtractDerivedFnTraits<ReturnType (C::*)(Args...) const volatile && noexcept> { 
    using type = void (C::*)(Args..., ReturnType*); 
  };
  #endif

  template<class F, class = void>
  struct is_function_pointer : std::false_type {};

  template<class F>
  struct is_function_pointer<F*, 
      typename std::enable_if<std::is_function<F>::value>::type> : std::true_type {};

  /**
   * \brief Compute type of derived function in forward differentiation mode 
   * using function, method or functor to be differentiated.
   * 
   */
  template<class F, class = void>
  struct ExtractDerivedFnTraitsForwMode {};

  /**
   * \brief Helper type for ExtractDerivedFnTraitsForwMode
   * 
   */
  template<class F>
  using ExtractDerivedFnTraitsForwMode_t = 
      typename ExtractDerivedFnTraitsForwMode<F>::type;

  template <class F>
  struct ExtractDerivedFnTraitsForwMode<
      F*,
      typename std::enable_if<std::is_class<F>::value>::type> {
    using type = decltype(&F::operator());
  };

  template <class F>
  struct ExtractDerivedFnTraitsForwMode<
      F,
      typename std::enable_if<is_function_pointer<F>::value>::type> {
    using type = F;
  };

  template <class F>
  struct ExtractDerivedFnTraitsForwMode<
      F,
      typename std::enable_if<std::is_member_function_pointer<F>::value>::type> {
    using type = F;
  };
  
  /**
   * \brief Compute class type from member function type, deduced type is 
   * void if free function type is provided. If class type is provided, 
   * then dedeuced type is same as that of provided class type.
   * 
   */
  template<class F, class = void>
  struct ExtractFunctorTraits {};

  template<class F>
  struct ExtractFunctorTraits<F*, 
      typename std::enable_if<std::is_class<F>::value>::type> {
    using type = F;
  };

  template<class F>
  struct ExtractFunctorTraits<F,
      typename std::enable_if<is_function_pointer<F>::value>::type> {
    using type = void;
  };

  template<class T, class C>
  struct ExtractFunctorTraits<T C::*> {
    using type = typename std::decay<C>::type;
  };
  /**
   * \brief Helper type for ExtractFunctorTraits
   */
  template<class F>
  using ExtractFunctorTraits_t = typename ExtractFunctorTraits<F>::type;

} // namespace clad

#endif // FUNCTION_TRAITS