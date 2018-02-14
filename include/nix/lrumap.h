#ifndef NIX_LRU_MAP_H_
#define NIX_LRU_MAP_H_

#include <nix/config.h>
#include <nix/assert.h>

//STL
#include <limits>
#include <unordered_map>
#include <list>

namespace nix{

	template<typename Key, typename T> using use_unordered_map = std::unordered_map<Key, T>;

    template<typename K, typename V, template<typename, typename> class Table = use_unordered_map>
    class lru_map{
    public:
        using key_type =  K;
        using value_type = V;

		using list_type = std::list<std::pair<K, V> >;
		using table_type = Table<K, typename list_type::iterator>;
	private:
        list_type m_items;
        table_type m_index;
        std::size_t m_cap;
        std::size_t m_hitted = 0;
        std::size_t m_missed = 0;
	public:
        
        explicit lru_map(std::size_t cap_ = 65536) : m_cap(cap_) {}
        
        std::size_t missed() const { return m_missed;}
        std::size_t hitted() const { return m_hitted;}
        std::size_t capacity() const { return m_cap;}
        std::size_t size() const { return m_items.size();}
        
        void capacity(std::size_t cap, list_type& result){
            XR_PRE_CONDITION(cap > 0);
            m_cap = cap;
            shrink_(result);
        }

        value_type& fetch(const K& key) {
            auto ret = try_fetch(key);
            XR_PRE_CONDITION(ret);
            return *ret;
        }
        
        value_type* try_fetch(const key_type& key){
            auto pos = m_index.find(key);
            if (pos == m_index.end()){
                ++m_missed;
                return nullptr;
            }
            
            ++m_hitted;
            m_items.splice(m_items.begin(), m_items, pos->second);
            return &pos->second->second;
        }

        void set(K key, V value, list_type& result) {
            set_(std::move(key), std::move(value), result);
		}
        list_type set(K key, V value) {
			list_type result;
			set_(std::move(key), std::move(value), result);
			return result;
		}

		list_type  erase(const K& key ){
			list_type res;
			erase(key, res);
			return res;
		}
		void erase(const K& key, list_type& res){
			auto pos = m_index.find(key);
			if (pos == m_index.end())	return ;

			m_items.erase(pos->second);
			res.splice(res.end(), m_items, pos->second);
			m_index.erase(pos);
		}

		typedef typename table_type::value_type index_value_type;
		template<typename UnaryPred>
		void erase_if(UnaryPred pred, list_type& res){
			for(auto itr = m_index.begin(); itr != m_index.end(); ){
				if (pred(*itr)){
					res.splice(res.end(), m_items, itr->second);
					itr = m_index.erase(itr);
				}
				else ++itr;
			}
		}

		template<typename F> void for_each(F f){
			for(auto&v : m_items)
				f(v);
		}
        
        //debug:
        const std::list<std::pair<K, V> >& get_items() const{ return m_items;}
        const std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator, H>& get_index() const { return m_index;}
    private:
        void set_(K key, V value, list_type& result) {
            auto pos = m_index.find(key);
            if (pos != m_index.end()){
                m_items.splice(m_items.begin(), m_items, pos->second);
                pos->second->second = std::move(value);
            }else{
                auto pnew = m_items.emplace(m_items.begin(), std::pair<K, V>(std::move(key), std::move(value)));
                m_index[key] = pnew;
            }
            
            shrink_(result);
        }

        void shrink_(list_type& res){
            while (m_index.size() > m_cap){
                auto rm = --m_items.end();
                m_index.erase(rm->first);
				res.splice(res.end(), m_items, rm);
            }
        }
        
    };
}

#endif //NIX_LRU_MAP_H_

