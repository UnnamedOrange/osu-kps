// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include <d2d1.h>
#include <d2d1_1.h>
#pragma comment(lib, "d2d1.lib")
#include <dwrite.h>
#pragma comment(lib, "dwrite.lib")

namespace d2d_helper
{
	/// <summary>
	/// 是否是 COM 类型。
	/// </summary>
	template <typename T>
	concept com = requires(T * obj)
	{
		std::derived_from<T, IUnknown>;
	};

	/// <summary>
	/// COM 类型智能指针。
	/// </summary>
	template <com com_t>
	class com_ptr
	{
		com_t* _p{};

	public:
		constexpr com_ptr() = default;
		com_ptr<com_t>& operator=(const com_ptr<com_t>& another)
		{
			if (this != &another)
			{
				_p = another._p;
				if (_p)
					_p->AddRef();
			}
			return *this;
		}
		com_ptr<com_t>& operator=(com_ptr<com_t>&& another) noexcept
		{
			if (this != &another)
			{
				_p = another._p;
				another._p = nullptr;
			}
			return *this;
		}
		com_ptr(const com_ptr<com_t>& another)
		{
			*this = another;
		}
		com_ptr(com_ptr<com_t>&& another) noexcept
		{
			*this = std::move(another);
		}
		virtual ~com_ptr()
		{
			reset();
		}

	public:
		com_t* get() const
		{
			return _p;
		}
		com_t** reset_and_get_address()
		{
			reset();
			return &_p;
		}
		void reset()
		{
			if (_p)
			{
				_p->Release();
				_p = nullptr;
			}
		}
		com_t* operator->() const
		{
			return _p;
		}
		operator bool() const
		{
			return _p;
		}
	};

	/// <summary>
	/// 自动创建并销毁工厂的单例对象。
	/// </summary>
	class factory
	{
		struct _RAII_factory
		{
			com_ptr<ID2D1Factory> d2d1_factory;
			com_ptr<IDWriteFactory> dwrite_factory;
			_RAII_factory()
			{
				if (FAILED(D2D1CreateFactory(
					D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_MULTI_THREADED, d2d1_factory.reset_and_get_address())))
					throw std::runtime_error("Fail to D2D1CreateFactory.");
				if (FAILED(DWriteCreateFactory(
					DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(IDWriteFactory),
					reinterpret_cast<IUnknown**>(dwrite_factory.reset_and_get_address()))))
					throw std::runtime_error("Fail to DWriteCreateFactory.");
			}
		};
		static _RAII_factory& singleton() // _factory 仅在使用时才构造。
		{
			static _RAII_factory _factory;
			return _factory;
		}

	public:
		/// <returns>ID2D1Factory 工厂。</returns>
		static ID2D1Factory* d2d1()
		{
			return singleton().d2d1_factory.get();
		}
		static IDWriteFactory* dwrite()
		{
			return singleton().dwrite_factory.get();
		}
	};
}