 
 
 
 
 

 
 

#ifndef CHAISCRIPT_EXCEPTION_SPECIFICATION_HPP_
#define CHAISCRIPT_EXCEPTION_SPECIFICATION_HPP_

#include <memory>

#include "../chaiscript_defines.hpp"
#include "boxed_cast.hpp"

namespace chaiscript {
  namespace detail {
    struct Exception_Handler_Base {
      virtual void handle(const Boxed_Value &bv, const Dispatch_Engine &t_engine) = 0;

      virtual ~Exception_Handler_Base() = default;

    protected:
      template<typename T>
      static void throw_type(const Boxed_Value &bv, const Dispatch_Engine &t_engine) {
        try {
          T t = t_engine.boxed_cast<T>(bv);
          throw t;
        } catch (const chaiscript::exception::bad_boxed_cast &) {
        }
      }
    };

    template<typename... T>
    struct Exception_Handler_Impl : Exception_Handler_Base {
      void handle(const Boxed_Value &bv, const Dispatch_Engine &t_engine) override { (throw_type<T>(bv, t_engine), ...); }
    };
  }  

   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
  using Exception_Handler = std::shared_ptr<detail::Exception_Handler_Base>;

   
   
  template<typename... T>
  Exception_Handler exception_specification() {
    return std::make_shared<detail::Exception_Handler_Impl<T...>>();
  }
}  

#endif
