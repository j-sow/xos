#ifndef PATH_ITERATOR_H
#define PATH_ITERATOR_H 1

#include <iostream> // XXX

#include <iterator>
#include <utility>
#include <vector>

#include "path.h"

namespace filesystem {
inline namespace v1 {

class path;

class path_iterator
  : public std::iterator<std::bidirectional_iterator_tag, path, void>
{
 public:
	enum class state
	{
		set_to_begin,
		set_to_end
	};

	typedef path::string_type::size_type offset_t;
	typedef std::pair<offset_t, offset_t> range;
	typedef std::vector<range> range_list;

	static constexpr offset_t npos = std::string::npos;

 private:
	static range_list build_elements(const path * p)
	{
		range_list elem;
		const std::string & s = p->generic_string();
		offset_t begin = 0, end = 0;

		if (! p->empty() )
		{
			// deal with any leading path separators
			if (s.at(begin) == path::preferred_separator)
			{
				end = 1;
				elem.push_back(std::make_pair(begin, end));
			}

			do {

				begin = s.find_first_not_of(path::preferred_separator, end);
				end = s.find_first_of(path::preferred_separator, begin);

				if (  (elem.size() != 1) || (! p->has_root_directory())
				   || (begin != npos) || (end != npos) )
				{
					elem.push_back(std::make_pair(begin, end));
				}
			} while (end != npos);
		}

		elem.shrink_to_fit();

#if 0
		for (auto & x : elem)
		{
			std::cout << "#---> (" << static_cast<int64_t>(x.first) << ", "
			          << static_cast<int64_t>(x.second) << ")\t";

			if (x.first != npos)
			{
				if (x.second != npos)
				{
					std::cout << s.substr(x.first, x.second - x.first);
				} else
				{
					std::cout << s.substr(x.first, s.length() - x.first);
				}
			} else
			{
				std::cout << ".";
			}
			std::cout << std::endl;
		}

		std::cout << "--- End of range build ---" << std::endl;
#endif

		return elem;
	}

 public:
	path_iterator()
	  : underlying(nullptr)
	  , elements()
	  , cursor(0)
	  , element_value()
		{ }

	path_iterator(const path * p, state _state = state::set_to_begin)
	  : underlying(p)
	  , elements(build_elements(underlying))
	  , cursor((_state == state::set_to_begin) ?
	           0 : elements.size())
	  , element_value()
		{ }

	path_iterator(const path_iterator & other)
	  : underlying(other.underlying)
	  , elements(other.elements)
	  , cursor(other.cursor)
	  , element_value(other.element_value)
		{ }

	path_iterator(path_iterator && other)
	  : underlying(std::move(other.underlying))
	  , elements(std::move(other.elements))
	  , cursor(other.cursor)
	  , element_value(std::move(other.element_value))
		{ }

	path_iterator & operator = (const path_iterator & other)
	{
		if (this != &other)
		{
			underlying = other.underlying;
			elements = other.elements;
			cursor = other.cursor;
			element_value = other.element_value;
		}
		return *this;
	}

	path_iterator & operator = (path_iterator && other)
	{
		if (this != &other)
		{
			underlying = std::move(other.underlying);
			elements = std::move(other.elements);
			cursor = other.cursor;
			element_value = std::move(other.element_value);
		}
		return *this;
	}

	~path_iterator() { }

	bool operator == (const path_iterator & other)
	{
		return (  (underlying == other.underlying)
		       && (cursor == other.cursor) );
	}

	bool operator != (const path_iterator & other)
	{
		return !(*this == other);
	}

	path_iterator & operator ++ ()
	{
		++cursor;
		return *this;
	}

	path_iterator operator ++ (int)
	{
		path_iterator tmp(*this);
		++(*this);
		return tmp;
	}

	path_iterator & operator -- ()
	{
		--cursor;
		return *this;
	}

	path_iterator operator -- (int)
	{
		path_iterator tmp(*this);
		--(*this);
		return tmp;
	}

	const path & operator * () const
	{
		create_element_value();
		return element_value;
	}

	const path * operator -> () const
	{
		create_element_value();
		return &element_value;
	}

 private:
	void create_element_value() const
	{
		range r = elements.at(cursor);
		const path::string_type & s = underlying->generic_string();

//		std::cout << "---> (" << static_cast<int64_t>(r.first) << ", "
//				  << static_cast<int64_t>(r.second) << ")\t";

		if (r.first != npos)
		{
			if (r.second != npos)
			{
				element_value = s.substr(r.first, r.second - r.first);
			} else
			{
				element_value = s.substr(r.first, s.length() - r.first);
			}
		} else
		{
			element_value = ".";
		}
	}

	const path * underlying;
	range_list elements;
	std::size_t cursor;
	mutable path element_value;
};


} // v1
} // filesystem

#endif // PATH_ITERATOR_H
