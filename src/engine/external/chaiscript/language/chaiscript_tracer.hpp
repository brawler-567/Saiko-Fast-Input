 
 
 
 
 

#ifndef CHAISCRIPT_TRACER_HPP_
#define CHAISCRIPT_TRACER_HPP_

namespace chaiscript::eval {
  struct Noop_Tracer_Detail {
    template<typename T>
    constexpr void trace(const chaiscript::detail::Dispatch_State &, const AST_Node_Impl<T> *) noexcept {
    }
  };

  template<typename... T>
  struct Tracer : T... {
    Tracer() = default;
    constexpr explicit Tracer(T... t)
        : T(std::move(t))... {
    }

    void do_trace(const chaiscript::detail::Dispatch_State &ds, const AST_Node_Impl<Tracer<T...>> *node) {
      (static_cast<T &>(*this).trace(ds, node), ...);
    }

    static void trace(const chaiscript::detail::Dispatch_State &ds, const AST_Node_Impl<Tracer<T...>> *node) {
      ds->get_parser().get_tracer<Tracer<T...>>().do_trace(ds, node);
    }
  };

  using Noop_Tracer = Tracer<Noop_Tracer_Detail>;

}  

#endif
