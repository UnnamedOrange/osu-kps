// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include <d2d1.h>
#include <d2d1_1.h>
#pragma comment(lib, "d2d1.lib")

namespace d2d_helper
{
	/// <summary>
	/// 自动创建并销毁工厂的单例对象。
	/// </summary>
	class factory
	{
		struct _RAII_factory
		{
			ID2D1Factory* d2d1_factory{};
			_RAII_factory()
			{
				if (FAILED(D2D1CreateFactory(
					D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_MULTI_THREADED, &d2d1_factory)))
					throw std::runtime_error("Fail to D2D1CreateFactory.");
			}
			~_RAII_factory()
			{
				if (d2d1_factory)
					d2d1_factory->Release();
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
			return singleton().d2d1_factory;
		}
	};

	/// <summary>
	/// 安全释放 ID2D 对象。
	/// </summary>
	/// <param name="p">对象指针的引用。</param>
	template <typename d2d_t>
	void release(d2d_t*& p)
	{
		if (p)
		{
			p->Release();
			p = nullptr;
		}
	}
}