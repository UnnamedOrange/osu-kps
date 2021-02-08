// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include <atomic>

#include <d2d1.h>
#include <d2d1helper.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <d2d1_3.h>
#include <d2d1_3helper.h>
#pragma comment(lib, "d2d1.lib")
#include <dwrite.h>
#include <dwrite_1.h>
#include <dwrite_2.h>
#include <dwrite_3.h>
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
		operator com_t* () const
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
			com_ptr<ID2D1Factory7> d2d1_factory;
			com_ptr<IDWriteFactory7> dwrite_factory;
			_RAII_factory()
			{
				if (FAILED(D2D1CreateFactory(
					D2D1_FACTORY_TYPE_MULTI_THREADED,
					__uuidof(ID2D1Factory7),
					reinterpret_cast<void**>(d2d1_factory.reset_and_get_address()))))
					throw std::runtime_error("Fail to D2D1CreateFactory.");
				if (FAILED(DWriteCreateFactory(
					DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(IDWriteFactory7),
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
		static auto d2d1()
		{
			return singleton().d2d1_factory;
		}
		static auto dwrite()
		{
			return singleton().dwrite_factory;
		}
	};

	/// <summary>
	/// 自定义 COM 对象。用于指明数据的生命周期。
	/// 实质上不存放数据！只是一个标记。
	/// 需要使用静态方法 create() 进行构造。
	/// 建议使用 com_ptr 进行管理。
	/// </summary>
	class life_indicator : public IUnknown // COM 对象。
	{
	private:
		std::atomic<ULONG> ref_count{ 1 };
	public:
		ULONG STDMETHODCALLTYPE AddRef() override
		{
			return ++ref_count;
		}
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void** object) override
		{
			*object = nullptr;
			if (riid == __uuidof(IUnknown))
			{
				AddRef();
				*object = this;
				return S_OK;
			}
			return E_NOINTERFACE;
		}
		ULONG STDMETHODCALLTYPE Release() override
		{
			--ref_count;
			if (!ref_count)
				delete this;
			return ref_count;
		}

	private:
		life_indicator() = default;
	public:
		static bool create(life_indicator** p)
		{
			return *p = new life_indicator;
		}
		life_indicator(const life_indicator&) = delete;
		life_indicator(life_indicator&&) = delete;
		life_indicator& operator=(const life_indicator&) = delete;
		life_indicator& operator=(life_indicator&&) = delete;
	};

	/// <summary>
	/// 私有字体序列。不要重复调用，否则会出现内存泄漏问题。
	/// </summary>
	class private_font_collection
	{
		inline static com_ptr<IDWriteInMemoryFontFileLoader> in_memory_font_file_loader;
		std::vector<BYTE> font;
		com_ptr<life_indicator> _life;
		com_ptr<IDWriteFontCollection1> collection;
		std::vector<std::wstring> family_names;
	public:
		private_font_collection() = default;
		private_font_collection(const std::vector<BYTE>& font_data)
		{
			from_font_data(font_data);
		}

	public:
		void from_font_data(const std::vector<BYTE>& font_data)
		{
			collection.reset();
			life_indicator::create(_life.reset_and_get_address());
			font = font_data;

			if (!in_memory_font_file_loader)
			{
				factory::dwrite()->CreateInMemoryFontFileLoader(in_memory_font_file_loader.reset_and_get_address());
				factory::dwrite()->RegisterFontFileLoader(in_memory_font_file_loader);
			}
			com_ptr<IDWriteFontSetBuilder2> font_set_builder;
			factory::dwrite()->CreateFontSetBuilder(font_set_builder.reset_and_get_address());
			{
				com_ptr<IDWriteFontFile> font_file_reference;
				in_memory_font_file_loader->CreateInMemoryFontFileReference(
					factory::dwrite(),
					font.data(),
					font.size(),
					_life.get(),
					font_file_reference.reset_and_get_address()
				);
				font_set_builder->AddFontFile(font_file_reference);
			}
			com_ptr<IDWriteFontSet> font_set;
			font_set_builder->CreateFontSet(font_set.reset_and_get_address());
			factory::dwrite()->CreateFontCollectionFromFontSet(font_set, collection.reset_and_get_address());

			family_names.clear();
			unsigned to = collection->GetFontFamilyCount();
			family_names.reserve(to);
			for (unsigned i = 0; i < to; i++)
			{
				com_ptr<IDWriteFontFamily> family;
				collection->GetFontFamily(i, family.reset_and_get_address());
				com_ptr<IDWriteLocalizedStrings> names;
				family->GetFamilyNames(names.reset_and_get_address());
				unsigned length{};
				names->GetStringLength(0, &length);
				std::wstring temp(length, L'\0');
				names->GetString(0, temp.data(), length + 1);
				family_names.emplace_back(std::move(temp));
			}
		}

	public:
		auto get() const
		{
			return collection.get();
		}
		const auto& get_family_names() const
		{
			return family_names;
		}
	};

	/// <summary>
	/// 颜色类。
	/// </summary>
	class color
	{
	public:
		FLOAT r{};
		FLOAT g{};
		FLOAT b{};
		FLOAT a{ 1 };
	public:
		static constexpr unsigned red_shift = 16;
		static constexpr unsigned green_shift = 8;
		static constexpr unsigned blue_shift = 0;
		static constexpr unsigned alpha_shift = 24;
		static constexpr unsigned red_mask = 0xff << red_shift;
		static constexpr unsigned green_mask = 0xff << green_shift;
		static constexpr unsigned blue_mask = 0xff << blue_shift;
		static constexpr unsigned alpha_mask = 0xff << alpha_shift;
	public:
		constexpr color() = default;
		constexpr color(unsigned int rgb_or_argb, unsigned char a = 0) :
			r{ static_cast<FLOAT>((rgb_or_argb & red_mask) >> red_shift) / 255.f },
			g{ static_cast<FLOAT>((rgb_or_argb & green_mask) >> green_shift) / 255.f },
			b{ static_cast<FLOAT>((rgb_or_argb & blue_mask) >> blue_shift) / 255.f }
		{
			if (a)
				this->a = static_cast<FLOAT>(a / 255.f);
			else
				this->a = static_cast<FLOAT>((rgb_or_argb & alpha_mask) >> alpha_shift) / 255.f;
		}
		constexpr color(FLOAT r, FLOAT g, FLOAT b, FLOAT a = 1.f) :
			r(r), g(g), b(b), a(a) {}
		constexpr color(unsigned r, unsigned g, unsigned b, unsigned a = 255) :
			r(static_cast<FLOAT>(r) / 255.f),
			g(static_cast<FLOAT>(g) / 255.f),
			b(static_cast<FLOAT>(b) / 255.f),
			a(static_cast<FLOAT>(a) / 255.f) {}
	public:
		static constexpr color linear_interpolation(const color& c0, const color& c1, FLOAT ratio)
		{
			color ret;
			ret.r = c0.r * (1 - ratio) + c1.r * ratio;
			ret.g = c0.g * (1 - ratio) + c1.g * ratio;
			ret.b = c0.b * (1 - ratio) + c1.b * ratio;
			ret.a = c0.a * (1 - ratio) + c1.a * ratio;
			return ret;
		}

	public:
		operator D2D1::ColorF() const
		{
			return D2D1::ColorF(r, g, b, a);
		}
	};
}