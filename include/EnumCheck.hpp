/**
 * @file EnumCheck.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of a constexpr enumeration value checker.
 * @version 0.1
 * @date 2025-03-25
 * 
 * The entirety of this code is copied from https://stackoverflow.com/a/33091821/2921426
 * 
 * @copyright Copyright (c) 2025 Simon Cahill.
 */

#ifndef LIBSOCKCANPP_INCLUDE_ENUMCHECK_HPP
#define LIBSOCKCANPP_INCLUDE_ENUMCHECK_HPP

namespace sockcanpp {

    template<typename EnumType, EnumType... Values>
    class EnumCheck;

    template<typename EnumType>
    class EnumCheck<EnumType> {
        public:
            template<typename IntType>
            static bool constexpr is_value(IntType) { return false; }
    };

    template<typename EnumType, EnumType V, EnumType... Next>
    class EnumCheck<EnumType, V, Next...> : private EnumCheck<EnumType, Next...> {
        using super = EnumCheck<EnumType, Next...>;

        public:
            template<typename IntType>
            static bool constexpr is_value(IntType v) {
                return v == static_cast<IntType>(V) || super::is_value(v);
            }
    };

}

#endif // LIBSOCKCANPP_INCLUDE_ENUMCHECK_HPP