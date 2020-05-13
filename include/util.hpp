#ifndef RS_UTIL_HPP
#define RS_UTIL_HPP

#include <cstddef>
#include <type_traits>
#include <tuple>

namespace rs {

template <typename... Ts>
struct print_types;

template<typename T> 
struct function_traits;  

template<typename R, typename ...Args> 
struct function_traits<R(*)(Args...)>
{
    static const size_t nargs = sizeof...(Args);
    using result_type = R;

    template <size_t i>
    struct arg
    {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
    };
};


template <typename Tp, typename... List >
struct contains : std::true_type {};

template <typename Tp, typename Head, typename... Rest >
struct contains<Tp, Head, Rest...>
: std::conditional< std::is_same<Tp, Head>::value,
    std::true_type,
    contains<Tp, Rest...> >::type {};

template <typename Tp >
struct contains<Tp> : std::false_type {};



template<class T, template<class...> class Template>
struct is_specialization : std::false_type {};

template<template<class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type {};

template<typename Test, template<size_t ...> class Ref>
struct is_specialization_num : std::false_type {};

template<template<size_t...> class Ref, size_t... Args>
struct is_specialization_num<Ref<Args...>, Ref> : std::true_type {};

}

#endif
