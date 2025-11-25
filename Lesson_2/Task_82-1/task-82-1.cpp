// Структурные шаблоны: Proxy, Decorator, Adapter. Задание
#include <iostream>
#include <string>
#include <algorithm>

class SimpleText
{
	public:
		virtual void render(const std::string& text)
		{
			std::cout << text;
		}
};

class DecoratedText :public SimpleText
{
	protected:
		SimpleText* __simple_text;
	public:
		DecoratedText(SimpleText* simple_text) : __simple_text(simple_text) {}
		
};

class ItalicText :public DecoratedText
{
	public:
	ItalicText(SimpleText* simple_text) :DecoratedText(simple_text) {}
	void render(const std::string& text) override
	{
		std::cout << "<i>";
		__simple_text->render(text);
		std::cout << "<i>";
	}
};

class BoldText :public DecoratedText
{
	public:
	BoldText(SimpleText* simple_text) :DecoratedText(simple_text) {}
	void render(const std::string& text) override
	{
		std::cout << "<b>";
		__simple_text->render(text);
		std::cout << "<b>";
	}
};

class Paragraph :public DecoratedText
{
	public:
		Paragraph(SimpleText* simple_text) :DecoratedText(simple_text) {}
	void render(const std::string& text) override
	{
		std::cout << "<p>";
		__simple_text->render(text);
		std::cout << "<p>";
	}

};

class Reversed :public DecoratedText
{
	public:
		Reversed(SimpleText* simple_text) :DecoratedText(simple_text) {}
	void render(const std::string& text) override
	{
		std::string str{text};
		std::reverse(str.begin(), str.end());
		__simple_text->render(str);
	}
};

class Link :public DecoratedText
{
	public:
		Link(SimpleText* simple_text) :DecoratedText(simple_text) {}
	void render(const std::string& text1, const std::string& text2)
	{
		std::cout <<"<a href=";
		__simple_text->render(text1);
		std::cout << ">";
		__simple_text->render(text2);
		std::cout << "</a>";
	}
};

int main()
{
	setlocale(LC_ALL,"Russian");
	std::cout << "Результаты рендеринга НTML:" << std::endl;
	auto text = new ItalicText(new BoldText(new SimpleText()));
	text->render(" Hello World ");

	std::cout << std::endl;
	auto text_block = new Paragraph(new SimpleText());
	text_block->render("Hello world");
	
	std::cout << std::endl;
	auto text_block2 = new Reversed(new SimpleText());
	text_block2->render("Hello world");

	std::cout << std::endl;
	auto text_block3 = new Link(new SimpleText());
	text_block3->render("netology.ru", "Hello world");
	std::cout << std::endl;

	return EXIT_SUCCESS;
}