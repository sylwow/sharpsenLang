#pragma once

#include <vector>
#include <utility>
#include <algorithm>

namespace sharpsenLang
{
	template <class Key, class Value>
	class Lookup
	{
	public:
		using ValueType = std::pair<Key, Value>;
		using ContainerType = std::vector<ValueType>;

	private:
		ContainerType _container;

	public:
		using Iterator = typename ContainerType::const_iterator;
		using ConstIterator = Iterator;

		Lookup(std::initializer_list<ValueType> init) : _container(init)
		{
			std::sort(_container.begin(), _container.end());
		}

		Lookup(container_type container) : _container(std::move(container))
		{
			std::sort(_container.begin(), _container.end());
		}

		ConstIterator begin() const
		{
			return _container.begin();
		}

		ConstIterator end() const
		{
			return _container.end();
		}

		template <class K>
		ConstIterator find(const K &key) const
		{
			ConstIterator it = std::lower_bound(
				begin(),
				end(),
				key,
				[](const ValueType &p, const K &key)
				{
					return p.first < key;
				});
			return it != end() && it->first == key ? it : end();
		}

		size_t size() const
		{
			return _container.size();
		}
	};
}
