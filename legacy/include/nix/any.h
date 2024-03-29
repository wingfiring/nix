//XIRANG2_LICENSE_PLACE_HOLDER

#ifndef XR_COMMON_ANY_H__
#define XR_COMMON_ANY_H__

#include <xirang2/exception.h>
#include <xirang2/context_except.h>

#include <typeinfo>
#include <cstring>
#include <type_traits>

namespace xirang2 {

    class any
    {
    public: // structors

        any() : content(0)
        {}
        
        template<typename ValueType>
        any(ValueType && value)
        : content(new holder<typename std::remove_cv<typename std::remove_reference<ValueType>::type>::type>
                  (std::move(value)))
        {}

        any(const any & other)
          : content(other.content ? other.content->clone() : 0)
        {}

        any(any && other)
          : content(other.content)
        {
            other.content = 0;
        }

        ~any()
        {
            check_delete(content);
        }

    public: // modifiers

        any & swap(any & rhs)
        {
            std::swap(content, rhs.content);
            return *this;
        }

        template<typename ValueType>
        any & operator=(const ValueType & rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

        any & operator=(const any& rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }
        any & operator=(any&& rhs)
        {
            rhs.swap(*this);
            return *this;
        }

    public: // queries

        bool empty() const
        {
            return !content;
        }

        const std::type_info & type() const
        {
            return content ? content->type() : typeid(void);
        }

    private: // types

        class placeholder
        {
        public: // structors

            virtual ~placeholder()
            {
            }

        public: // queries

            virtual const std::type_info & type() const = 0;

            virtual placeholder * clone() const = 0;

        };

        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors

            holder(const ValueType & value)
              : held(value)
            {
            }
            
            holder(ValueType && value)
            : held(std::move(value))
            {
            }

        public: // queries

            virtual const std::type_info & type() const
            {
                return typeid(ValueType);
            }

            virtual placeholder * clone() const
            {
                return new holder(held);
            }

        public: // representation

            ValueType held;

        private: // intentionally left unimplemented
            holder & operator=(const holder &);
        };

    private: // representation

        template<typename ValueType>
        friend ValueType * any_cast(any *);

        template<typename ValueType>
        friend ValueType * unsafe_any_cast(any *);

        placeholder * content;

    };

    XR_EXCEPTION_TYPE(bad_any_cast);

    template<typename ValueType>
    ValueType * any_cast(any * operand)
    {
        return operand && 
			(operand->type() == typeid(ValueType)
			 || std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0
			)
			 ? &static_cast<any::holder<ValueType> *>(operand->content)->held
            : 0;
    }

    template<typename ValueType>
    inline const ValueType * any_cast(const any * operand)
    {
        return any_cast<ValueType>(const_cast<any *>(operand));
    }

    template<typename ValueType>
    ValueType any_cast(any & operand)
    {
        typedef typename std::remove_reference<ValueType>::type nonref;

        nonref * result = any_cast<nonref>(&operand);
        if(!result)
            XR_THROW(bad_any_cast);
        return *result;
    }

    template<typename ValueType>
    inline ValueType any_cast(const any & operand)
    {
        typedef typename std::remove_reference<ValueType>::type nonref;

        return any_cast<const nonref &>(const_cast<any &>(operand));
    }

    // Note: The "unsafe" versions of any_cast are not part of the
    // public interface and may be removed at any time. They are
    // required where we know what type is stored in the any and can't
    // use typeid() comparison, e.g., when our types may travel across
    // different shared libraries.
    template<typename ValueType>
    inline ValueType * unsafe_any_cast(any * operand)
    {
        return &static_cast<any::holder<ValueType> *>(operand->content)->held;
    }

    template<typename ValueType>
    inline const ValueType * unsafe_any_cast(const any * operand)
    {
        return unsafe_any_cast<ValueType>(const_cast<any *>(operand));
    }
}

// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#endif
