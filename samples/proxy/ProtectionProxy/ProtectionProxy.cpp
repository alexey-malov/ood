#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

enum class Role
{
	Guest,
	User,
	Admin
};

struct User
{
	string name;
	Role role = Role::Guest;
};

// Интерфейс документа
struct IDocument
{
	virtual void Read() const = 0;
	virtual void Edit(const string& newText) = 0;
	virtual void Delete() = 0;
	virtual ~IDocument() = default;
};

// Реальный документ (без проверки прав)
class Document : public IDocument
{
public:
	explicit Document(string text)
		: m_text(move(text))
	{
	}

	void Read() const override
	{
		cout << "Document content: " << m_text << endl;
	}

	void Edit(const string& newText) override
	{
		cout << "Editing document...\n";
		m_text = newText;
	}

	void Delete() override
	{
		cout << "Deleting document...\n";
		m_text.clear();
	}

private:
	string m_text;
};

// Protection Proxy
class DocumentProtectionProxy : public IDocument
{
public:
	DocumentProtectionProxy(shared_ptr<IDocument> realDoc, User currentUser)
		: m_realDoc(move(realDoc))
		, m_user(move(currentUser))
	{
	}

	void Read() const override
	{
		// Чтение доступно всем
		m_realDoc->Read();
	}

	void Edit(const string& newText) override
	{
		if (!HasWriteAccess())
		{
			cout << "Access denied: user '" << m_user.name
				 << "' is not allowed to edit the document\n";
			return;
		}
		m_realDoc->Edit(newText);
	}

	void Delete() override
	{
		if (!HasAdminAccess())
		{
			cout << "Access denied: user '" << m_user.name
				 << "' is not allowed to delete the document\n";
			return;
		}
		m_realDoc->Delete();
	}

private:
	bool HasWriteAccess() const
	{
		return m_user.role == Role::User || m_user.role == Role::Admin;
	}

	bool HasAdminAccess() const
	{
		return m_user.role == Role::Admin;
	}

	shared_ptr<IDocument> m_realDoc;
	User m_user;
};

int main()
{
	auto realDoc = make_shared<Document>("Top secret data");

	User guest{ "Alice", Role::Guest };
	User editor{ "Bob", Role::User };
	User admin{ "Charlie", Role::Admin };

	DocumentProtectionProxy guestDoc(realDoc, guest);
	DocumentProtectionProxy editorDoc(realDoc, editor);
	DocumentProtectionProxy adminDoc(realDoc, admin);

	cout << "--- Guest ---\n";
	guestDoc.Read();
	guestDoc.Edit("New text"); // нет прав на редактирование
	guestDoc.Delete(); // нет прав на удаление

	cout << "\n--- Editor ---\n";
	editorDoc.Read();
	editorDoc.Edit("Edited by editor"); // можно редактировать
	editorDoc.Delete(); // удалять нельзя

	cout << "\n--- Admin ---\n";
	adminDoc.Read();
	adminDoc.Edit("Edited by admin");
	adminDoc.Delete(); // админ может всё

	return 0;
}
