// Порождающие шаблоны. Задание 1
#include <assert.h>
#include "sql_query_builder.h"

int main()
{
	SqlSelectQueryBuilder query_builder;
	query_builder.AddColumn("name").AddColumn("phone");
	query_builder.AddFrom("students");
	query_builder.AddWhere("id", "42").AddWhere("name", "John");

	assert(query_builder.BuildQuery() == "SELECT name, phone FROM students WHERE id=42 AND name=John;");
	std::cout << "Query: " << query_builder.BuildQuery() << std::endl;
	
	return EXIT_SUCCESS;
}