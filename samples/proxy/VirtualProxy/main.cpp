#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <fstream>

using namespace std;

struct Size
{
	Size() = default;
	Size(int w, int h) : width(w), height(h) {}
	int width = 0;
	int height = 0;
};

class IImage
{
public:
	virtual Size GetSize()const = 0;
	virtual void Draw() const = 0;
	virtual ~IImage() = default;
};

class CImage : public IImage
{
public:
	CImage(const vector<uint8_t> & /*data*/)
	{
		// Декодируем изображение
	}

	void Draw() const override
	{
		cout << "Drawing an image\n";
	}

	Size GetSize() const override
	{
		return m_size;
	}
private:
	Size m_size;
	// Прочие данные
};

// Выполняет загрузку URL-а. Уведомляет об окончании загрузки, вызывая callback
void LoadImageFromURL(const string & url, 
	const function<void(unique_ptr<IImage>&&)> & callback)
{
	cout << "Loading image from URL " << url << endl;
	ifstream strm(url, ios_base::binary);
	if (strm)
	{
		strm.seekg(0, ios_base::end);
		auto size = strm.tellg();
		vector<uint8_t> buf(static_cast<size_t>(size));
		strm.seekg(0, ios_base::beg);
		if (size > 0 && strm.read(reinterpret_cast<char*>(buf.data()), size))
		{
			callback(make_unique<CImage>(buf));
			return;
		}
	}
	callback(nullptr);
}

class CImageProxy : public IImage
{
	struct Impl : enable_shared_from_this<Impl>
	{
		Impl(const Size & size, const string & url)
			: m_size(size)
			, m_url(url)
		{

		}
		Size GetSize() const
		{
			if (m_image)
			{
				// предполагаем, что размеры одинаковые
				return m_image->GetSize();
			}
			return m_size;
		}

		void Draw() const
		{
			if (!m_image && !m_isLoading && !m_loadingError)
			{
				m_isLoading = true;

				weak_ptr<const Impl> weakSelf = shared_from_this();
				LoadImageFromURL(m_url, [=](auto && image) {
					if (auto strongSelf = weakSelf.lock())
					{
						m_isLoading = false;
						m_image = move(image);
						m_loadingError = !m_image;
						if (m_loadingError)
							cout << "Image loading has failed\n";
						else
							cout << "Image has been loaded successfully\n";
					}
					else
					{
						// объект уже разрушен, просто игнорируем полученное изображение
						// Использовать данные нельзя
					}
				});
			}

			if (m_image)
			{
				m_image->Draw();
			}
			else if (m_isLoading)
			{
				cout << "Drawing a loading indicator\n";
				// Рисуем индикатор загрузки
			}
			else if (m_loadingError)
			{
				cout << "Drawing 'broken image' icon\n";
				// Рисуем "сломанное" изображение
			}
		}
	private:
		string m_url;
		Size m_size;
		mutable bool m_loadingError = false;
		mutable bool m_isLoading = false;
		mutable unique_ptr<IImage> m_image;
	};
public:
	CImageProxy(const Size & size, const string & url)
		: m_impl(make_shared<Impl>(size, url))
	{
	}

	Size GetSize() const override
	{
		return m_impl->GetSize();
	}

	void Draw() const override
	{
		m_impl->Draw();
	}
private:
	shared_ptr<Impl> m_impl;
};

void WorkWithImage(IImage & image)
{
	auto size = image.GetSize();
	cout << "Image size:" << size.width << "x" << size.height << endl;
	image.Draw();
}

int main()
{
	CImageProxy img({30, 30}, "img1.png");
	WorkWithImage(img);
	return 0;
}