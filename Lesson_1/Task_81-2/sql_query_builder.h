#ifndef SQL_QUERY_BUILDER_H
#define SQL_QUERY_BUILDER_H

#include <iostream>
#include <vector>
#include <map>

// Класс для формирования SELECT-запросов
class SqlSelectQueryBuilder
{
private:
	std::string table_name;
	std::vector<std::string> column;
	std::string where_condition;

public:

	// Метод для выбора столбцов для SQL запроса
	SqlSelectQueryBuilder& AddColumn(const std::string name_column)
	{
		column.push_back(name_column);
		return *this;
	}

	// Метод для выбора таблиц для SQL запроса
	SqlSelectQueryBuilder& AddFrom(const std::string table)
	{
		table_name = table;
		return *this;
	}

	// Метод для определения условий SQL запроса
	SqlSelectQueryBuilder& AddWhere(const std::string prizn, const std::string condition)
	{
		std::string str_where{prizn + "=" + condition};
		if (!where_condition.empty())
		{
			where_condition += " AND " + str_where;
		}
		else
		{
			where_condition = str_where;
		}
		return *this;
	}

	// Метод BuildQuery
	std::string BuildQuery()
	{
		if (table_name.empty())
		{
			throw std::runtime_error("Не задана таблица!!");
		}

		std::string query = "SELECT ";

		if (column.empty())
		{
			query += "*";
		}
		else
		{
			for (size_t i{}; i < column.size(); ++i)
			{
				query += column[i] + (i == column.size() - 1 ? "" : ", ");
			}
		}

		query += " FROM " + table_name;

		if (!where_condition.empty())
		{
			query += " WHERE " + where_condition;
		}

		query += ";";

		return query;
	}

	// Метод для определения условий SQL запроса
	SqlSelectQueryBuilder& AddWhere(const std::map<std::string, std::string>& kv) noexcept
	{
		std::string str_where{};
		if (!kv.empty())
		{
			for (const auto& elem : kv)
			{
				str_where = elem.first + "=" + elem.second;
				if (!where_condition.empty())
				{
					where_condition += " AND " + str_where;
				}
				else
				{
					where_condition = str_where;
				}
			}
		}
		return *this;
	}

	// Метод для выбора столбцов для SQL запроса
	SqlSelectQueryBuilder& AddColumns(const std::vector<std::string>& columns) noexcept
	{

		if (!columns.empty())
		{
			for (size_t i{}; i < columns.size(); ++i)
			{
				column.push_back(columns[i]);
			}
		}
		return *this;
	}
};

#endif 

