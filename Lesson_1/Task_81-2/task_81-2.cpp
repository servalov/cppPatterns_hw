// Порождающие шаблоны. Задание 2
#include <assert.h>
#include "sql_query_builder.h"

int main()
{
	SqlSelectQueryBuilder query_builder;
	query_builder.AddColumns({ "name", "phone" });
	query_builder.AddFrom("students");
	query_builder.AddWhere({ {"id", "42"},{"name", "John"} });

	assert(query_builder.BuildQuery() == "SELECT name, phone FROM students WHERE id=42 AND name=John;");
	std::cout << "Query: " << query_builder.BuildQuery() << std::endl;

	return EXIT_SUCCESS;
}