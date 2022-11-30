#pragma once

#include <functional>
#include <assert.h>
#include <vector>
#include <iterator>

namespace hsk{

inline size_t next_power_of_two(size_t i)
{
    --i;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i |= i >> 32;
    ++i;
    return i;
}


template<typename K, typename V, typename H = std::hash<K>>
class hashmap{

struct Bucket{
	static constexpr int8_t def_dist = -1;
	int8_t dist = def_dist;
	K _key; V _value;

	Bucket() = default;
	Bucket(const K & key, const V & value) : dist(0), _key(key), _value(value){}
	Bucket(const K & key) : _key(key) {}
	Bucket(K && key, V && value) : _key(std::move(key)), _value(std::move(value)) {}
	Bucket(K && key) : _key(std::move(key)) {}

	void swap(Bucket & other){
		using std::swap;
		swap(_key, other._key);
		swap(_value, other._value);
		swap(dist, other.dist);
	}

	~Bucket() = default;
};

using Iterator = std::vector<Bucket>::iterator;

private:
	size_t _size, _filled;
	static constexpr size_t def_siz = 1<<3;
	H _hash;

	std::vector<Bucket> _table;

	void Realloc(size_t new_size){
		_size = new_size;
		
		std::vector<Bucket> _newtable(_size);

		std::swap(_table, _newtable);

		for(auto &b : _newtable){
			if(b.dist != -1){
				emplace(b);
			}
		}

		_newtable.~vector();
	}

	inline void next_iterator (Iterator &it) { 
		if(++it == std::end(_table)){
			it = std::begin(_table);
		}
	}

	inline Iterator next_itr (Iterator it) const{
		return (++it == std::end(_table) ? std::begin(_table) : it);
	}

	inline size_t bucket_id (const K & key) const{
		return _hash(key)&_size;
	}

	Iterator emplace(Bucket &b, bool ok = 1){
		Iterator it = std::begin(_table);
		std::advance(it, bucket_id(b._key));
		++_filled;


		while(1){
			if(b.dist > it->dist){
				std::swap(b, *it);
				if(b.dist == -1){
					break;
				}
			}else if(b.dist == it->dist && b._key == it->_key){
				--_filled;
				if(ok) std::swap(b, *it);
				break;
			}
			b.dist++;
			next_iterator(it);
		}

		if((_filled<<1) > _size){
			Realloc(_size << 1);
		}
		return it;
	}

public:
	hashmap(size_t sz = def_siz) : _filled(0){

		assert(sz > 0);
		
		// sz must be power of 2.
		_size = next_power_of_two(sz);
		_table.resize(_size);
	}

	template<typename Key, typename Value>
	void emplace(Key &&key, Value &&value){
		Bucket b(key, value);
		emplace(b);
	}

	Iterator find(const K &key){
		Iterator it = std::begin(_table);
		std::advance(it, bucket_id(key));
		// Iterator it = std::advance(begin(_table), bucket_id(key));
		int8_t dist = 0;

		while(1){
			if(dist > it->dist) break;

			if(dist == it->dist && it->_key == key) return it;

			++dist;
			next_iterator(it);
		}

		return end(_table);
	}

	void erase(const K & key) {
		Iterator it = find(key);
		if(it != end(_table)){
			--_filled;
			*it = Bucket();
			while(1){
				auto nit = next_itr(it);

				if(nit->dist < 1) break;
				swap(*it, *nit);

				it = nit;
			}
		}
	}

	size_t size() const
    {
        return _filled;
    }

	void clear(){
		_table.clear();
	}

	V& operator [] (const K & key){
		Bucket b(key);
		return emplace(b, false)->_value;
	}

	inline bool empty(){
		return (_filled == 0);
	}

	~hashmap() = default;
};

}


