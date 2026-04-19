 
 
 
 
 

 
 

#ifndef CHAISCRIPT_REGISTER_FUNCTION_HPP_
#define CHAISCRIPT_REGISTER_FUNCTION_HPP_

#include <type_traits>

#include "bind_first.hpp"
#include "function_signature.hpp"
#include "proxy_functions.hpp"

namespace chaiscript {
  namespace dispatch::detail {
    template<typename Obj, typename Param1, typename... Rest>
    Param1 get_first_param(Function_Params<Param1, Rest...>, Obj &&obj) {
      return static_cast<Param1>(std::forward<Obj>(obj));
    }

    template<typename Func, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject, bool Is_Object, typename Ret, typename... Param>
    auto make_callable_impl(Func &&func, Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, Is_Object>) {
      if constexpr (Is_MemberObject) {
         
        return Proxy_Function(chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Attribute_Access<Ret, std::decay_t<Param>...>>(
            std::forward<Func>(func)));
      } else if constexpr (Is_Member) {
         
        auto call = [func = std::forward<Func>(func)](auto &&obj, auto &&...param)   -> decltype(auto) {
          return ((get_first_param(Function_Params<Param...>{}, obj).*func)(std::forward<decltype(param)>(param)...));
        };
        return Proxy_Function(
            chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret(Param...), decltype(call)>>(
                std::move(call)));
      } else {
        return Proxy_Function(
            chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret(Param...), std::decay_t<Func>>>(
                std::forward<Func>(func)));
      }
    }

     
     
    template<typename Func, typename Ret, typename Object, typename... Param, bool Is_Noexcept>
    auto make_callable(Func &&func, Function_Signature<Ret, Function_Params<Object, Param...>, Is_Noexcept, false, false, true>) {
      return make_callable_impl(std::forward<Func>(func), Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, false, false, true>{});
    }

    template<typename Func, typename Ret, typename... Param, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject>
    auto make_callable(Func &&func, Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, false> fs) {
      return make_callable_impl(std::forward<Func>(func), fs);
    }
  }  

   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  template<typename T>
  Proxy_Function fun(T &&t) {
    return dispatch::detail::make_callable(std::forward<T>(t), dispatch::detail::function_signature(t));
  }

   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  template<typename T, typename Q>
  Proxy_Function fun(T &&t, const Q &q) {
    return fun(detail::bind_first(std::forward<T>(t), q));
  }

}  

#endif
