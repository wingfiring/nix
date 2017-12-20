#ifndef XR_COMMON_XIRANG2_BINDER_H__
#define XR_COMMON_XIRANG2_BINDER_H__

#include <xirang2/type/object.h>
#include <xirang2/type/itypebinder.h>

#include <xirang2/context_except.h>


namespace xirang2 { namespace type{
    ///  exception for type mismatch
	XR_EXCEPTION_TYPE(TypeMismatchException);

	enum TypeMismatchPolicy{
		tmp_disallow,
		tmp_exception,
		tmp_discard,
        tmp_nocheck,
	};
	template<typename T, typename CommonObjType>
	class ObjectRefBase{
		public:
		typedef T value_type;
		typedef typename std::remove_const<T>::type raw_type;

		T& operator*() const{
			XR_PRE_CONDITION(target_.valid());
			return *static_cast<T*>(target_.data());
		}

		T* operator->() const{
			XR_PRE_CONDITION(target_.valid());
			return static_cast<T*>(target_.data());
		}

		ObjectRefBase() {}
		ObjectRefBase(CommonObjType target, TypeMismatchPolicy tmc){
			XR_PRE_CONDITION(target.valid());
			switch (tmc){
				case tmp_disallow:
					{
					XR_PRE_CONDITION(target.type().version() == TypeVersionOf<raw_type>::value);
					target_ = target;
					}
					break;
				case tmp_exception:
					/// XXX: compare version might be slow, if so,  we can compare only first 4 bytes
					if(target.type().version() != TypeVersionOf<raw_type>::value)
						XR_THROW(TypeMismatchException);
					target_ = target;
					break;
				case tmp_discard:
					if(target.type().version() == TypeVersionOf<raw_type>::value)
						target_ = target;
					break;
                case tmp_nocheck:
                    target_ = target;
                    break;
			};
		}
        T* get() const { return valid() ? static_cast<T*>(target_.data()) : nullptr; }
		bool valid() const{ return target_.valid();}
		private:
			CommonObjType target_;
	};

	template<typename T, typename CommonObjType>
	class ObjectRefModelBase{
		public:
		typedef T value_type;
		typedef typename std::remove_const<T>::type raw_type;

		T operator*() const{
			XR_PRE_CONDITION(target_.valid());
			return T(target_);
		}

		ObjectRefModelBase() {}
		ObjectRefModelBase(CommonObjType target, TypeMismatchPolicy tmc){
			XR_PRE_CONDITION(target.valid());
			switch (tmc){
				case tmp_disallow:
					XR_PRE_CONDITION(target.type().model().version() == TypeVersionOf<raw_type>::value);
					target_ = target;
					break;
				case tmp_exception:
					/// XXX: compare version might be slow, if so,  we can compare only first 4 bytes
					if(target.type().model().version() != TypeVersionOf<raw_type>::value)
						XR_THROW(TypeMismatchException);
					target_ = target;
					break;
				case tmp_discard:
					if(target.type().model().version() == TypeVersionOf<raw_type>::value)
						target_ = target;
					break;
                case tmp_nocheck:
                    target_ = target;
                    break;
			};
		}

		bool valid() const{ return target_.valid();}
		private:
			CommonObjType target_;
	};

	template<typename T>
	struct ObjectRef : public ObjectRefBase<T, CommonObject>{
		ObjectRef() {}
		ObjectRef(CommonObject target, TypeMismatchPolicy tmc = tmp_disallow)
			: ObjectRefBase<T, CommonObject>(target, tmc)
		{}
	};

	template<typename T>
	struct ConstObjectRef : public ObjectRefBase<const T, ConstCommonObject>{
		ConstObjectRef() {}
		ConstObjectRef(ConstCommonObject target, TypeMismatchPolicy tmc = tmp_disallow)
			: ObjectRefBase<const T, ConstCommonObject>(target, tmc)
		{}
	};

#define SPECIALIZE_OBJECT_REF(Type, Base)\
	template<>\
	struct ObjectRef <Type>: public Base<Type, CommonObject>{\
		ObjectRef() {}\
		ObjectRef(CommonObject target, TypeMismatchPolicy tmc = tmp_disallow)\
			: Base<Type, CommonObject>(target, tmc)\
		{}\
	};

#define SPECIALIZE_CONST_OBJECT_REF(Type, Base)\
	template<>\
	struct ConstObjectRef<Type> : public Base<const Type, ConstCommonObject>{\
		ConstObjectRef() {}\
		ConstObjectRef(ConstCommonObject target, TypeMismatchPolicy tmc = tmp_disallow)\
			: Base<const Type, ConstCommonObject>(target, tmc)\
		{}\
	};\

#define SPECIALIZE_OBJECT_REF_ALL(Type, Base)\
	SPECIALIZE_OBJECT_REF(Type, Base)\
	SPECIALIZE_CONST_OBJECT_REF(Type, Base)

	template<typename T>
	ObjectRef<T> Ref(CommonObject obj, TypeMismatchPolicy tmc = tmp_disallow){
		return ObjectRef<T>(obj, tmc);
	}
	template<typename T>
	ConstObjectRef<T> Ref(ConstCommonObject obj, TypeMismatchPolicy tmc = tmp_disallow){
		return ConstObjectRef<T>(obj, tmc);
	}

	template<typename T>
	ObjectRef<T> UncheckRef(CommonObject obj){
		return ObjectRef<T>(obj, tmp_nocheck);
	}
	template<typename T>
	ConstObjectRef<T> UncheckRef(ConstCommonObject obj){
		return ConstObjectRef<T>(obj, tmp_nocheck);
	}

    /// Bind T* to const CommonObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T > T * bind (const CommonObject * obj)
	{
		XR_PRE_CONDITION(obj && obj->valid());

		Type t = obj->type();
		XR_PRE_CONDITION(t.valid());

		return TypeVersionOf<T>::value == t.version()
			? reinterpret_cast<T*>(obj->data())
			: 0;
	}

    /// Bind T& to const CommonObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T > T & bind (const CommonObject & obj)
	{
		XR_PRE_CONDITION(obj.valid());

		T* p = bind<T>(&obj);

		if (!p)
			XR_THROW(TypeMismatchException);
		return *p;
	}


    /// Bind T* to const CommonObject pointer. no type check.
    /// \pre obj && obj->valid()
	template < typename T > T * uncheckBind (const CommonObject * obj)
	{
		XR_PRE_CONDITION(obj && obj->valid());

		return reinterpret_cast<T*>(obj->data());
	}

    /// Bind T& to const CommonObject&. no type check.
    /// \pre obj->valid()
	template < typename T > T & uncheckBind (const CommonObject & obj)
	{
		XR_PRE_CONDITION(obj.valid());

		return *reinterpret_cast<T*>(obj.data());
	}

    /// Bind T* to const SubObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T > T * bind (const SubObject * obj)
	{
		XR_PRE_CONDITION(obj && obj->valid());

		Type t = obj->type();
		XR_PRE_CONDITION(t.valid());

        return TypeVersionOf<typename std::remove_const<T>::type>::value == t.version()
			? reinterpret_cast<T*>(obj->data())
			: 0;
	}

    /// Bind T& to const SubObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T > T & bind (const SubObject & obj)
	{
		XR_PRE_CONDITION(obj.valid());

		T* p = bind<T>(&obj);

		if (!p)
		{
			XR_THROW(TypeMismatchException);
		}
		return *p;
	}

    /// Bind T* to const SubObject pointer. no type check.
    /// \pre obj && obj->valid()
	template < typename T > T * uncheckBind (const SubObject * obj)
	{
		XR_PRE_CONDITION(obj && obj->valid());

		return reinterpret_cast<T*>(obj->data());
	}

    /// Bind T& to const SubObject&. no type check.
    /// \pre obj->valid()
	template < typename T > T & uncheckBind (const SubObject & obj)
	{
		XR_PRE_CONDITION(obj.valid());

		return *reinterpret_cast<T*>(obj.data());
	}


    /// Bind const T* to const ConstCommonObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T > const T * bind (const ConstCommonObject * obj)
	{
		XR_PRE_CONDITION(obj && obj->valid());

		Type t = obj->type();
		XR_PRE_CONDITION(t.valid());

		return TypeVersionOf<typename std::remove_const<T>::type>::value == t.version()
			? reinterpret_cast< const T*>(obj->data())
			: 0;
	}

    /// Bind const T& to const ConstCommonObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T > const T & bind (const ConstCommonObject & obj)
	{
		XR_PRE_CONDITION(obj.valid());

		const T* p = bind<T>(&obj);

		if (!p)
		{
			XR_THROW(TypeMismatchException);
		}
		return *p;
	}

    /// Bind const T* to const ConstCommonObject pointer. no type check.
    /// \pre obj && obj->valid()
	template < typename T > const T * uncheckBind (const ConstCommonObject * obj)
	{
		XR_PRE_CONDITION(obj && obj->valid());

		return reinterpret_cast<const T*>(obj->data());
	}

    /// Bind const T& to const ConstCommonObject&. no type check.
    /// \pre obj->valid()
	template < typename T > const T & uncheckBind (const ConstCommonObject & obj)
	{
		XR_PRE_CONDITION(obj.valid());

		return *reinterpret_cast<const T*>(obj.data());
	}

    /// Bind const T* to const ConstSubObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T > const T * bind (const ConstSubObject * obj)
	{
		XR_PRE_CONDITION(obj && obj->valid());

		Type t = obj->type();
		XR_PRE_CONDITION(t.valid());

		return TypeVersionOf<T>::value == t.version()
			? reinterpret_cast< const T*>(obj->data())
			: 0;
	}

    /// Bind const T& to const ConstSubObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T > const T & bind (const ConstSubObject & obj)
	{
		XR_PRE_CONDITION(obj.valid());

		const T* p = bind<T>(&obj);

		if (!p)
		{
			XR_THROW(TypeMismatchException);
		}
		return *p;
	}

    /// Bind const T* to const ConstSubObject pointer. no type check.
    /// \pre obj && obj->valid()
	template < typename T > const T * uncheckBind (const ConstSubObject * obj)
	{
		XR_PRE_CONDITION(obj && obj->valid());

		return reinterpret_cast<const T*>(obj->data());
	}

    /// Bind T& to const SubObject&. no type check.
    /// \pre obj->valid()
	template < typename T > const T & uncheckBind (const ConstSubObject & obj)
	{
		XR_PRE_CONDITION(obj.valid());

		return *reinterpret_cast<const T*>(obj.data());
	}

#ifdef _DEBUG
#define SELECTED_BIND_FUNCTION bind
#else
#define SELECTED_BIND_FUNCTION uncheckBind
#endif
    /// Bind T* to const CommonObject pointer. if type mismatch, it returns null pointer.
    /// \pre obj && obj->valid()
	template < typename T, typename ObjType > T * fastBind (const ObjType * obj)
	{
        return SELECTED_BIND_FUNCTION(obj);
	}

    /// Bind T& to const CommonObject refrence. if type mismatch, it throws TypeMismatchException .
    /// \pre obj->valid()
	template < typename T, typename ObjType >  T & fastBind (const ObjType & obj)
	{
        return SELECTED_BIND_FUNCTION(obj);
	}
#undef SELECTED_BIND_FUNCTION

}}
#endif				//end XR_COMMON_XIRANG2_BINDER_H__
