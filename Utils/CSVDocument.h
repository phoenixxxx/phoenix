#pragma once
#include "Types.h"
#include <initializer_list>
#include <fstream>
namespace Phoenix
{
	template <std::size_t ColCount>
	class CSVDocument
	{
	public:
		void Create(cstr_t name, std::initializer_list<cstr_t> columns)
		{
			mFile.open(name);
			std::size_t range = std::min(ColCount, columns.size());
			std::size_t iCol = 0;
			for (auto col : columns)
			{
				if (iCol == range)
					break;
				mColumns[iCol++] = col;
				mFile << col << ",";
			}
			mFile << std::endl;
		}

		void Close()
		{
			mFile.close();
		}

		template <typename T, typename... Args>
		void Write(T a0, Args... args)
		{
			Write(a0);
			Write(args...);
			mFile << std::endl;
		}

		template <typename T>
		void Write(T argument)
		{
			mFile << argument << ",";
		}

	private:
		stdstr_t mColumns[ColCount];
		std::ofstream mFile;
	};
}
