#ifndef COMPOSITION_HPP
#define COMPOSITION_HPP

#include <utility>

namespace rs {

template <class G, class F>
constexpr auto compose(G &&g, F &&f) {
    return [g,f](auto ... args) { return g(f(args...)); };
}

template <class G, class F, class ... Hs>
constexpr auto compose(G &&g, F &&f, Hs&& ... hs) {
    return compose(std::forward<G>(g),compose(std::forward<F>(f), std::forward<Hs>(hs)...));
}

template <typename Trafo>
struct trafo_helper {
  Trafo trafo;
  // umesto ovog je funkcija ispod
  template <typename ... Args>
  auto operator()(Args&& ...args) { return trafo(std::forward<Args...>(args)...); }
};

template <typename Trafo>
constexpr auto transform(Trafo &&trafo) {
  return trafo_helper<Trafo>{std::forward<Trafo>(trafo)};
}

template <class F, class G>
constexpr auto operator | (const trafo_helper<F> &&f, const trafo_helper<G> &&g) {
    return transform(compose(g.trafo, f.trafo));
}

}

#endif // COMPOSITION_HPP
