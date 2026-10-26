#define NOMINMAX
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

using namespace std;

struct Stats {
    int books = 0;
    int issued = 0;
    int available = 0;
    int readers = 0;
    int readersWithBooks = 0;
};

int readInt(const wstring& prompt, int minValue = 0) {
    int value;
    while (true) {
        wcout << prompt;
        if (wcin >> value && value >= minValue) {
            wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
            return value;
        }
        wcout << L"Ошибка ввода. Введите целое число не меньше "
            << minValue << L".\n";
        wcin.clear();
        wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
    }
}

wstring readText(const wstring& prompt, bool canSkip = false) {
    wstring value;
    while (true) {
        wcout << prompt;
        getline(wcin, value);
        if (canSkip && value.empty()) return value;
        if (!value.empty()) return value;
        wcout << L"Поле не должно быть пустым.\n";
    }
}

class Book;

class Reader {
private:
    int id;
    wstring fullName;
    wstring phone;
    vector<Book*> books;

public:
    Reader(int id, const wstring& name, const wstring& phone)
        : id(id), fullName(name), phone(phone) {
    }

    int getId() const {
        return id;
    }

    wstring getFullName() const {
        return fullName;
    }

    wstring getPhone() const {
        return phone;
    }

    int getBooksCount() const {
        return static_cast<int>(books.size());
    }

    const vector<Book*>& getBooks() const {
        return books;
    }

    void setFullName(const wstring& name) {
        if (!name.empty()) fullName = name;
    }

    void setPhone(const wstring& value) {
        if (!value.empty()) phone = value;
    }

    void addBook(Book* book) {
        books.push_back(book);
    }

    void removeBook(Book* book) {
        for (auto it = books.begin(); it != books.end(); ++it) {
            if (*it == book) {
                books.erase(it);
                return;
            }
        }
    }

    void print() const {
        wcout << L"Билет №" << id
            << L" | " << fullName
            << L" | телефон: " << phone
            << L" | книг на руках: " << books.size() << L'\n';
    }
};

class Book {
private:
    int id;
    wstring title;
    wstring author;
    int year;
    shared_ptr<Reader> owner;

public:
    Book(int id, const wstring& title,
        const wstring& author, int year)
        : id(id), title(title), author(author), year(year) {
    }

    int getId() const {
        return id;
    }

    wstring getTitle() const {
        return title;
    }

    wstring getAuthor() const {
        return author;
    }

    int getYear() const {
        return year;
    }

    bool isIssued() const {
        return owner != nullptr;
    }

    shared_ptr<Reader> getOwner() const {
        return owner;
    }

    void setTitle(const wstring& value) {
        if (!value.empty()) title = value;
    }

    void setAuthor(const wstring& value) {
        if (!value.empty()) author = value;
    }

    void setYear(int value) {
        if (value > 0) year = value;
    }

    void issueTo(const shared_ptr<Reader>& reader) {
        owner = reader;
    }

    void giveBack() {
        owner.reset();
    }

    void print() const {
        wcout << L"№" << id
            << L" | «" << title << L"»"
            << L" | " << author
            << L" | " << year << L" | ";

        if (owner)
            wcout << L"выдана: " << owner->getFullName() << L'\n';
        else
            wcout << L"в наличии\n";
    }
};

class Library {
private:
    vector<unique_ptr<Book>> books;
    vector<shared_ptr<Reader>> readers;

    int nextBookId = 1;
    int nextReaderId = 1;

    Book* findBook(int id) const {
        for (const auto& book : books) {
            if (book->getId() == id)
                return book.get();
        }
        return nullptr;
    }

    shared_ptr<Reader> findReader(int id) const {
        for (const auto& reader : readers) {
            if (reader->getId() == id)
                return reader;
        }
        return nullptr;
    }

public:
    void addBook() {
        wstring title = readText(L"Название: ");
        wstring author = readText(L"Автор: ");
        int year = readInt(L"Год издания: ", 1);

        books.push_back(
            make_unique<Book>(nextBookId++, title, author, year)
        );

        wcout << L"Книга добавлена.\n";
    }

    void showBooks(int mode = 0) const {
        bool found = false;

        for (const auto& book : books) {
            if (mode == 1 && !book->isIssued()) continue;
            if (mode == 2 && book->isIssued()) continue;

            book->print();
            found = true;
        }

        if (!found)
            wcout << L"Список пуст.\n";
    }

    void editBook() {
        int id = readInt(L"Номер книги для изменения: ", 1);
        Book* book = findBook(id);

        if (!book) {
            wcout << L"Книга не найдена.\n";
            return;
        }

        wcout << L"Оставьте поле пустым, чтобы не менять его.\n";

        wstring title = readText(L"Новое название: ", true);
        wstring author = readText(L"Новый автор: ", true);
        int year = readInt(L"Новый год (0 — не менять): ", 0);

        book->setTitle(title);
        book->setAuthor(author);

        if (year > 0)
            book->setYear(year);

        wcout << L"Данные книги изменены.\n";
    }

    void deleteBook() {
        int id = readInt(L"Номер книги для удаления: ", 1);

        for (auto it = books.begin(); it != books.end(); ++it) {
            if ((*it)->getId() == id) {
                if ((*it)->isIssued()) {
                    wcout << L"Нельзя удалить выданную книгу. "
                        << L"Сначала верните её.\n";
                    return;
                }

                books.erase(it);
                wcout << L"Книга удалена, память освобождена.\n";
                return;
            }
        }

        wcout << L"Книга не найдена.\n";
    }

    void addReader() {
        wstring name = readText(L"ФИО читателя: ");
        wstring phone = readText(L"Телефон: ");

        readers.push_back(
            make_shared<Reader>(nextReaderId++, name, phone)
        );

        wcout << L"Читатель добавлен.\n";
    }

    void showReaders() const {
        if (readers.empty()) {
            wcout << L"Список читателей пуст.\n";
            return;
        }

        for (const auto& reader : readers)
            reader->print();
    }

    void editReader() {
        int id = readInt(L"Номер билета читателя: ", 1);
        auto reader = findReader(id);

        if (!reader) {
            wcout << L"Читатель не найден.\n";
            return;
        }

        wcout << L"Оставьте поле пустым, чтобы не менять его.\n";

        wstring name = readText(L"Новое ФИО: ", true);
        wstring phone = readText(L"Новый телефон: ", true);

        reader->setFullName(name);
        reader->setPhone(phone);

        wcout << L"Данные читателя изменены.\n";
    }

    void deleteReader() {
        int id = readInt(L"Номер билета для удаления: ", 1);

        for (auto it = readers.begin(); it != readers.end(); ++it) {
            if ((*it)->getId() == id) {
                if ((*it)->getBooksCount() > 0) {
                    wcout << L"Нельзя удалить читателя, "
                        << L"пока у него есть книги.\n";
                    return;
                }

                readers.erase(it);
                wcout << L"Читатель удалён, память освобождена.\n";
                return;
            }
        }

        wcout << L"Читатель не найден.\n";
    }

    void issueBook() {
        int bookId = readInt(L"Номер книги: ", 1);
        Book* book = findBook(bookId);

        if (!book) {
            wcout << L"Книга не найдена.\n";
            return;
        }

        if (book->isIssued()) {
            wcout << L"Книга уже выдана.\n";
            return;
        }

        int readerId = readInt(L"Номер билета читателя: ", 1);
        auto reader = findReader(readerId);

        if (!reader) {
            wcout << L"Читатель не найден.\n";
            return;
        }

        book->issueTo(reader);
        reader->addBook(book);

        wcout << L"Книга выдана читателю.\n";
    }

    void returnBook() {
        int id = readInt(L"Номер книги для возврата: ", 1);
        Book* book = findBook(id);

        if (!book) {
            wcout << L"Книга не найдена.\n";
            return;
        }

        if (!book->isIssued()) {
            wcout << L"Эта книга уже находится в библиотеке.\n";
            return;
        }

        auto reader = book->getOwner();
        reader->removeBook(book);
        book->giveBack();

        wcout << L"Книга возвращена.\n";
    }

    void showReaderBooks() const {
        int id = readInt(L"Номер билета читателя: ", 1);
        auto reader = findReader(id);

        if (!reader) {
            wcout << L"Читатель не найден.\n";
            return;
        }

        wcout << L"Книги читателя "
            << reader->getFullName() << L":\n";

        if (reader->getBooks().empty()) {
            wcout << L"Книг на руках нет.\n";
            return;
        }

        for (const Book* book : reader->getBooks())
            book->print();
    }

    void showStatistics() const {
        Stats* stats = new Stats;

        stats->books = static_cast<int>(books.size());
        stats->readers = static_cast<int>(readers.size());

        for (const auto& book : books) {
            if (book->isIssued())
                ++stats->issued;
            else
                ++stats->available;
        }

        for (const auto& reader : readers) {
            if (reader->getBooksCount() > 0)
                ++stats->readersWithBooks;
        }

        wcout << L"\n--- Статистика библиотеки ---\n"
            << L"Всего книг: " << stats->books << L'\n'
            << L"Выдано книг: " << stats->issued << L'\n'
            << L"Книг в наличии: " << stats->available << L'\n'
            << L"Всего читателей: " << stats->readers << L'\n'
            << L"Читателей с книгами: "
            << stats->readersWithBooks << L'\n';

        delete stats;
        stats = nullptr;

        if (!readers.empty()) {
            int n = static_cast<int>(readers.size());
            int* counts = new int[n];
            int maxIndex = 0;

            for (int i = 0; i < n; ++i) {
                counts[i] = readers[i]->getBooksCount();

                if (counts[i] > counts[maxIndex])
                    maxIndex = i;
            }

            wcout << L"\nКоличество книг у читателей:\n";

            for (int i = 0; i < n; ++i) {
                wcout << readers[i]->getFullName()
                    << L" — " << counts[i] << L'\n';
            }

            wcout << L"Больше всего книг у читателя: "
                << readers[maxIndex]->getFullName()
                << L" (" << counts[maxIndex] << L")\n";

            delete[] counts;
            counts = nullptr;
        }
    }

    void fillStartData() {
        readers.push_back(make_shared<Reader>(
            nextReaderId++,
            L"Бочков Михаил Сергеевич",
            L"+7-900-111-11-11"
        ));

        readers.push_back(make_shared<Reader>(
            nextReaderId++,
            L"Пупкин Степан Олегович",
            L"+7-900-222-33-44"
        ));

        readers.push_back(make_shared<Reader>(
            nextReaderId++,
            L"Филиппов Финес Сергеевич",
            L"+7-900-333-44-55"
        ));

        books.push_back(make_unique<Book>(
            nextBookId++,
            L"Преступление и наказание",
            L"Ф. М. Достоевский",
            1866
        ));

        books.push_back(make_unique<Book>(
            nextBookId++,
            L"Мёртвые души",
            L"Н. В. Гоголь",
            1842
        ));

        books.push_back(make_unique<Book>(
            nextBookId++,
            L"Война и мир",
            L"Л. Н. Толстой",
            1869
        ));

        books.push_back(make_unique<Book>(
            nextBookId++,
            L"Тихий Дон",
            L"М. А. Шолохов",
            1940
        ));

        books.push_back(make_unique<Book>(
            nextBookId++,
            L"Язык программирования C++",
            L"Т. А. Мурашов",
            2026
        ));

        books.push_back(make_unique<Book>(
            nextBookId++,
            L"Архитектура апаратных средств",
            L"Д. Е. Морозиков",
            2026
        ));

        books[0]->issueTo(readers[0]);
        readers[0]->addBook(books[0].get());

        books[4]->issueTo(readers[0]);
        readers[0]->addBook(books[4].get());

        books[1]->issueTo(readers[1]);
        readers[1]->addBook(books[1].get());
    }
};

void setupConsole() {
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
}

void showMenu() {
    wcout << L"\n========== УПРАВЛЕНИЕ БИБЛИОТЕКОЙ ==========\n"
        << L"1. Работа с книгами\n"
        << L"2. Работа с читателями\n"
        << L"3. Выдать книгу читателю\n"
        << L"4. Вернуть книгу\n"
        << L"5. Показать книги читателя\n"
        << L"6. Статистика библиотеки\n"
        << L"0. Выход\n";
}

void booksMenu(Library& library) {
    while (true) {
        wcout << L"\n--- КНИГИ ---\n"
            << L"1. Показать все книги\n"
            << L"2. Показать выданные\n"
            << L"3. Показать книги в наличии\n"
            << L"4. Добавить книгу\n"
            << L"5. Изменить книгу\n"
            << L"6. Удалить книгу\n"
            << L"0. Назад\n";

        int choice = readInt(L"Выберите действие: ", 0);

        if (choice == 0) return;
        if (choice == 1) library.showBooks();
        else if (choice == 2) library.showBooks(1);
        else if (choice == 3) library.showBooks(2);
        else if (choice == 4) library.addBook();
        else if (choice == 5) library.editBook();
        else if (choice == 6) library.deleteBook();
        else wcout << L"Нет такого пункта меню.\n";
    }
}

void readersMenu(Library& library) {
    while (true) {
        wcout << L"\n--- ЧИТАТЕЛИ ---\n"
            << L"1. Показать читателей\n"
            << L"2. Добавить читателя\n"
            << L"3. Изменить читателя\n"
            << L"4. Удалить читателя\n"
            << L"0. Назад\n";

        int choice = readInt(L"Выберите действие: ", 0);

        if (choice == 0) return;
        if (choice == 1) library.showReaders();
        else if (choice == 2) library.addReader();
        else if (choice == 3) library.editReader();
        else if (choice == 4) library.deleteReader();
        else wcout << L"Нет такого пункта меню.\n";
    }
}

int main() {
    setupConsole();

    Library library;
    library.fillStartData();

    int choice;

    do {
        showMenu();
        choice = readInt(L"Выберите действие: ", 0);

        if (choice == 1) booksMenu(library);
        else if (choice == 2) readersMenu(library);
        else if (choice == 3) library.issueBook();
        else if (choice == 4) library.returnBook();
        else if (choice == 5) library.showReaderBooks();
        else if (choice == 6) library.showStatistics();
        else if (choice != 0)
                wcout << L"Нет такого пункта меню.\n";

    } while (choice != 0);

    wcout << L"Работа программы завершена.\n";
    return 0;
}