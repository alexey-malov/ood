#include "GoF-iterator.h"

#include <deque>
#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Book
{
public:
	Book(string title, string author, int publicationYear)
		: m_title(std::move(title))
		, m_author(std::move(author))
		, m_publicationYear(publicationYear)
	{
	}
	string GetTitle() const
	{
		return m_title;
	}
	string GetAuthor() const
	{
		return m_author;
	}
	int GetPublicationYear() const
	{
		return m_publicationYear;
	}

private:
	string m_title;
	string m_author;
	int m_publicationYear;
};

ostream& operator<<(ostream& out, const Book& book)
{
	out << std::format(R"("{}" by {}, {})",
		book.GetTitle(), book.GetAuthor(), book.GetPublicationYear());
	return out;
}

#if 0
typedef IIterator<const Book> IConstBookIterator;

class IBooks
{
public:
	virtual unique_ptr<IConstBookIterator> CreateIterator() const = 0;

	virtual ~IBooks() = default;
};

class Library : public IBooks
{
public:
	typedef unique_ptr<Book> BookPtr;

	void AddBook(const Book& book)
	{
		m_books.emplace_back(make_unique<Book>(book));
	}

	size_t GetBooksCount() const
	{
		return m_books.size();
	}

	Book const& GetBookAtIndex(size_t index) const
	{
		return *m_books.at(index);
	}

	unique_ptr<IConstBookIterator> CreateIterator() const override;

private:
	vector<BookPtr> m_books;
};

template <typename ValueType, typename IteratorType>
class MapValuePtrIterator : public IIterator<ValueType>
{
public:
	MapValuePtrIterator(const IteratorType& begin, const IteratorType& end)
		: m_begin(begin)
		, m_end(end)
	{
	}

	bool HasNext() const override
	{
		return m_begin != m_end;
	}

	ValueType& Get() const override
	{
		return *(m_begin->second);
	}

	void Next() override
	{
		++m_begin;
	}

	unique_ptr<IIterator<ValueType>> Clone() const override
	{
		return make_unique<MapValuePtrIterator>(*this);
	}

private:
	IteratorType m_begin;
	IteratorType m_end;
};

template <typename T, typename IteratorType>
auto MakeMapValuePtrIterator(const IteratorType& begin, const IteratorType& end)
{
	return make_unique<MapValuePtrIterator<T, IteratorType>>(begin, end);
}

class BookCatalog : public IBooks
{
	using BooksColl = std::unordered_map<string, Book>;

public:
	void AddBook(const Book& book)
	{
		m_books.emplace(book.GetTitle(), book);
	}

	unique_ptr<IConstBookIterator> CreateIterator() const override
	{
		return MakeMapValuePtrIterator<const Book>(m_books.cbegin(), m_books.cend());
	}

private:
	BooksColl m_books;
};


void PrintLibraryBooks(const Library& library)
{
	for (size_t i = 0; i < library.GetBooksCount(); ++i)
	{
		cout << library.GetBookAtIndex(i) << endl;
	}
}

void PrintCatalogBooks(const BookCatalog& catalog)
{
	auto& books = catalog.GetBooks();
	for (auto& titleAndBook : books)
	{
		cout << *titleAndBook.second << endl;
	}
}

class CConstLibraryBookIterator : public IConstBookIterator
{
public:
	CConstLibraryBookIterator(const Library& library)
		: m_library(library)
	{
	}

	bool HasNext() const override
	{
		return m_index < m_library.GetBooksCount();
	}

	const Book& Get() const override
	{
		return m_library.GetBookAtIndex(m_index);
	}

	void Next() override
	{
		if (!HasNext())
		{
			throw logic_error("Can't move next");
		}
		++m_index;
	}

	unique_ptr<IConstBookIterator> Clone() const override
	{
		return make_unique<CConstLibraryBookIterator>(*this);
	}

private:
	const Library& m_library;
	size_t m_index = 0;
};

unique_ptr<IConstBookIterator> Library::CreateIterator() const
{
	return make_unique<CConstLibraryBookIterator>(*this);
}

void PrintBooks(const IBooks& books)
{
	auto it = books.CreateIterator();
	while (it->HasNext())
	{
		cout << it->Get() << endl;
		it->Next();
	}
}

void PrintBooks(CIteratorWrapper<const Book> it)
{
	while (it.HasNext())
	{
		cout << it.Get() << endl;
		it.Next();
	}
}
int main()
{
	Library lib;
	lib.AddBook(Book("Moby Dick", "Herman Melville", 1851));
	lib.AddBook(Book("The Adventures of Tom Sawyer", "Mark Twain", 1876));
	lib.AddBook(Book("Crime and Punishment", "Fyodor Dostoevsky", 1866));
	lib.AddBook(Book("White Fang", "Jack London", 1906));
	lib.AddBook(Book("The Hound of the Baskervilles", "Arthur Conan Doyle", 1902));

	BookCatalog catalog;
	catalog.AddBook(Book("Moby Dick", "Herman Melville", 1851));
	catalog.AddBook(Book("The Adventures of Tom Sawyer", "Mark Twain", 1876));
	catalog.AddBook(Book("Crime and Punishment", "Fyodor Dostoevsky", 1866));
	catalog.AddBook(Book("White Fang", "Jack London", 1906));
	catalog.AddBook(Book("The Hound of the Baskervilles", "Arthur Conan Doyle", 1902));

	cout << "==== Books from Library ====" << endl;
	PrintLibraryBooks(lib);
	cout << endl
		 << "==== Catalog books sorted by title ====" << endl;
	PrintCatalogBooks(catalog);

	cout << endl
		 << "==== Books library books using iterator ====" << endl;
	PrintBooks(lib);
	cout << endl
		 << "==== Catalog books sorted by title uning iterator ====" << endl;
	PrintBooks(catalog);

	cout << endl
		 << "==== Books library books using iterator ====" << endl;
	PrintBooks(lib.CreateIterator());
	cout << endl
		 << "==== Catalog books sorted by title uning iterator ====" << endl;
	PrintBooks(catalog.CreateIterator());
	return 0;
}

#else

#if 0
class Library
{
public:
	using BookByTitle = std::unordered_map<std::string, const Book*>;

	void AddBook(Book book)
	{
		const auto& b = m_books.emplace_back(std::move(book));
		auto& authorBooks = m_authorBooks[b.GetAuthor()];
		if (!authorBooks.try_emplace(b.GetTitle(), &b).second)
		{
			m_books.pop_back();
			throw std::invalid_argument("Book already exists");
		}
	}

	size_t GetBookCount() const noexcept
	{
		return m_books.size();
	}

	const Book& GetBookAtIndex(size_t index) const
	{
		return m_books.at(index);
	}

	const BookByTitle* FindAuthorBooks(const string& author) const
	{
		auto it = m_authorBooks.find(author);
		return it != m_authorBooks.end() ? &it->second : nullptr;
	}

	const Book* FindBook(const string& author, const string& title) const
	{
		auto* authorBooks = FindAuthorBooks(author);
		if (!authorBooks)
			return nullptr;
		auto it = authorBooks->find(title);
		return it != authorBooks->end() ? it->second : nullptr;
	}

private:
	std::deque<Book> m_books;
	std::unordered_map<std::string, BookByTitle> m_authorBooks;
};

void PrintBooks(const Library& library)
{
	size_t n = library.GetBookCount();
	for (size_t i = 0; i < n; ++i)
	{
		std::cout << library.GetBookAtIndex(i) << std::endl;
	}
}

void PrintBooks(const Library::BookByTitle& books)
{
	for (auto& [_, book] : books)
	{
		std::cout << *book << std::endl;
	}
}
#endif

#if 1

class IBookIterator
{
public:
	virtual ~IBookIterator() = default;
	virtual const Book& GetBook() const = 0;
	virtual bool HasBook() const = 0;
	virtual std::unique_ptr<IBookIterator> Clone() const = 0;
	virtual void Next() = 0;
};

class Library
{
public:
	void AddBook(Book book)
	{
		const auto& b = m_books.emplace_back(std::move(book));
		auto& authorBooks = m_authorBooks[b.GetAuthor()];
		if (!authorBooks.try_emplace(b.GetTitle(), &b).second)
		{
			m_books.pop_back();
			throw std::invalid_argument("Book already exists");
		}
	}

	size_t GetBookCount() const noexcept
	{
		return m_books.size();
	}

	const Book& GetBookAtIndex(size_t index) const
	{
		return m_books.at(index);
	}

	std::unique_ptr<IBookIterator> GetAllBooks() const
	{
		return std::make_unique<DequeBookIterator>(m_books.begin(), m_books.end());
	}

	std::unique_ptr<IBookIterator> FindAuthorBooks(const string& author) const
	{
		auto it = m_authorBooks.find(author);
		return it != m_authorBooks.end()
			? make_unique<BookByTitleIterator>(it->second.begin(), it->second.end())
			: make_unique<BookByTitleIterator>();
	}

	const Book* FindBook(const string& author, const string& title) const
	{
		auto authorBooksIt = m_authorBooks.find(author);
		if (authorBooksIt == m_authorBooks.end())
			return nullptr;
		auto titleToBookIt = authorBooksIt->second.find(title);
		return titleToBookIt != authorBooksIt->second.end() ? titleToBookIt->second : nullptr;
	}

private:
	using BookByTitle = std::unordered_map<std::string, const Book*>;

	class DequeBookIterator : public IBookIterator
	{
	public:
		using Coll = std::deque<Book>;
		using Iter = Coll::const_iterator;
		DequeBookIterator() = default;
		DequeBookIterator(Iter current, Iter end)
			: m_current(current)
			, m_end(end)
		{
		}
		const Book& GetBook() const override
		{
			return HasBook() ? *m_current : throw std::logic_error("Invalid iterator");
		}
		bool HasBook() const override
		{
			return m_current != m_end;
		}
		void Next() override
		{
			if (!HasBook())
				throw std::logic_error("Invalid iterator");
			++m_current;
		}
		std::unique_ptr<IBookIterator> Clone() const override
		{
			// Вызываем конструктор копирования
			return std::make_unique<DequeBookIterator>(*this);
		}

	private:
		Iter m_current;
		Iter m_end;
	};
	class BookByTitleIterator : public IBookIterator
	{
	public:
		using Iter = BookByTitle::const_iterator;
		BookByTitleIterator() = default;
		BookByTitleIterator(Iter current, Iter end)
			: m_current(current)
			, m_end(end)
		{
		}
		// Inherited via IBookIterator
		const Book& GetBook() const override
		{
			return HasBook() ? *m_current->second : throw std::logic_error("Invalid iterator");
		}
		bool HasBook() const override
		{
			return m_current != m_end;
		}
		void Next() override
		{
			if (!HasBook())
				throw std::logic_error("Invalid iterator");
			++m_current;
		}
		std::unique_ptr<IBookIterator> Clone() const override
		{
			// Вызываем конструктор копирования
			return std::make_unique<BookByTitleIterator>(*this);
		}

	private:
		Iter m_current;
		Iter m_end;
	};

	std::deque<Book> m_books;
	std::unordered_map<std::string, BookByTitle> m_authorBooks;
};

class BookIterator
{
public:
	BookIterator(unique_ptr<IBookIterator> it)
		: m_it(it ? std::move(it) : throw std::invalid_argument("Invalid iterator"))
	{
	}
	BookIterator(const BookIterator& other)
		: m_it(other.m_it->Clone())
	{
	}
	BookIterator& operator=(const BookIterator& rhs)
	{
		if (this != &rhs)
		{
			m_it = rhs.m_it->Clone();
		}
		return *this;
	}

	const Book& GetBook() const { return m_it->GetBook(); }
	bool HasBook() const { return m_it->HasBook(); }
	void Next() { m_it->Next(); }

private:
	unique_ptr<IBookIterator> m_it;
};

void PrintBooks(BookIterator iter)
{
	for (; iter.HasBook(); iter.Next())
	{
		std::cout << iter.GetBook() << std::endl;
	}
}

#endif

int main()
{
	Library library;
	library.AddBook(Book{ "Moby Dick", "Herman Melville", 1851 });
	library.AddBook(Book{ "White Fang", "Jack London", 1906 });
	library.AddBook(Book{ "Harry Potter and the Philosopher's Stone", "Joanne Rowling", 1997 });
	library.AddBook(Book{ "Harry Potter and the Chamber of Secrets", "Joanne Rowling", 1998 });
	PrintBooks(library.GetAllBooks());
	if (auto joanneRowlingBooks = library.FindAuthorBooks("Joanne Rowling"))
	{
		std::cout << "---" << std::endl;
		PrintBooks(std::move(joanneRowlingBooks));
	}
}

#endif
