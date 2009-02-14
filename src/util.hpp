#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <string>
#include <cstring>
#include <cstddef>
#include <cstdarg>
#include <iterator>
#include <stdexcept>

template <typename T, std::size_t size> class array
{
	private:
		T data[size];

	public:
		array()
		{
			memset((void *)this->data, 0, sizeof(T) * size);
		}

		array(T init[size])
		{
			std::memcpy((void *)this->data, (void *)init, sizeof(T) * size);
		}

		T &operator[](std::size_t index)
		{
			if (index < 0 || index >= size)
			{
				throw std::out_of_range("Out of range accessing array.");
			}

			return this->data[index];
		}
};

typedef array<unsigned char, 2> pairchar;
typedef array<unsigned char, 4> quadchar;

std::string ltrim(const std::string &);
std::string rtrim(const std::string &);
std::string trim(const std::string &);

#endif // UTIL_HPP_INCLUDED
