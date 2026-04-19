 
 
 
 
 

 
 

#ifndef CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_
#define CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_

#include "proxy_functions.hpp"

namespace chaiscript::dispatch::detail {
  template<typename Class, typename... Params>
  Proxy_Function build_constructor_(Class (*)(Params...)) {
    if constexpr (!std::is_copy_constructible_v<Class>) {
      auto call = [](auto &&...param) { return std::make_shared<Class>(std::forward<decltype(param)>(param)...); };

      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base,
                                  dispatch::Proxy_Function_Callable_Impl<std::shared_ptr<Class>(Params...), decltype(call)>>(call));
    } else if constexpr (true) {
      auto call = [](auto &&...param) { return Class(std::forward<decltype(param)>(param)...); };

      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Class(Params...), decltype(call)>>(
              call));
    }
  }
}  

namespace chaiscript {
   
   
   
   
   
   
   
   
   
   
   
  template<typename T>
  Proxy_Function constructor() {
    T *f = nullptr;
    return (dispatch::detail::build_constructor_(f));
  }
}  

#endif
