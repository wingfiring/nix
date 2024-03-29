#ifndef XR_COMMON_PATH_H__
#define XR_COMMON_PATH_H__
#include <xirang2/string.h>
#include <xirang2/string_algo/utf8.h>
#include <xirang2/string_algo/string.h>
#include <xirang2/operators.h>

#include <xirang2/utility/make_reverse_iterator.h> //for make_reverse_iterator

namespace xirang2{
    
    enum path_process{
        pp_none,
        pp_normalize = 1,
        pp_utf8check = 2,
        pp_winfile = 4,
        pp_convert_backslash = 8,
        pp_default = 0x0b,      // ~pp_winfile
        pp_localfile = pp_default | pp_winfile,	//
        
    };
    
    class XR_API sub_file_path : totally_ordered<sub_file_path>
    {
    public:
        static const char dim;
        sub_file_path();
        explicit sub_file_path(const_range_string str);
        
        explicit sub_file_path(string::const_iterator first,
                               string::const_iterator last);
        sub_file_path& operator=(const sub_file_path& rhs);
        void swap(sub_file_path& rhs);
        
        sub_file_path parent() const;
        sub_file_path ext() const;
        sub_file_path stem() const;
        sub_file_path filename() const;
        sub_file_path version() const;
        sub_file_path exclude_version() const;
        sub_file_path pure_filename() const;
        sub_file_path pop_front(size_t n = 1) const;
        sub_file_path pop_back(size_t n = 1) const;
        
        bool is_absolute() const;
        bool is_network() const;
        bool is_root() const;
        bool has_disk() const;
        bool is_pure_disk() const;
        bool is_normalized() const;
        bool empty() const;
        bool has_version() const;
        
        bool under(sub_file_path path) const;
        bool contains(sub_file_path path) const;
        
        const_range_string str() const;
        string native_str() const;
        wstring native_wstr() const;
        wstring wstr() const;
        
        class iterator;
        typedef iterator const_iterator;
        
        iterator begin() const;
        iterator end() const;
    private:
        const_range_string m_str;
    };
    
    inline bool operator<(const sub_file_path& lhs, const sub_file_path& rhs){
        return lhs.str() < rhs.str();
    }
    inline bool operator==(const sub_file_path& lhs, const sub_file_path& rhs){
        return lhs.str() == rhs.str();
    }
    
    // a/b ==> a : b
    // /a/b ==> / : a : b
    // //a/b ==> //a : / : b
    // ///a/b ==> / : a : b
    class XR_API sub_file_path::iterator{
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef sub_file_path 	value_type;
        typedef std::ptrdiff_t	difference_type;
        typedef const sub_file_path*	pointer;
        typedef const sub_file_path&	reference;
        
        iterator();
        void swap(iterator& rhs);
        
        iterator& operator++();
        iterator& operator--();
        iterator operator++(int);
        iterator operator--(int);
        
        bool operator==(const iterator& rhs);
        bool operator!=(const iterator& rhs);
        
        reference operator*() const;
        pointer operator->() const;
        
    private:
        string::const_iterator begin_() const;
        string::const_iterator end_() const;
        sub_file_path path_() const;
        iterator(const_range_string path, string::const_iterator pos);
        
        friend class sub_file_path;
        string::const_iterator m_pos;
        const_range_string m_path;
        mutable sub_file_path m_cache;
    };
    
    class XR_API  file_path : totally_ordered<file_path>
    {
    public:
        typedef sub_file_path::iterator iterator;
        typedef sub_file_path::const_iterator const_iterator;
        
        file_path();
        file_path(const sub_file_path& str);
        
        explicit file_path(const string& str, path_process pp = pp_default);
        explicit file_path(const wstring& str, path_process pp = pp_default);
        file_path(const file_path& rhs);
        file_path(file_path&& rhs);
        
        file_path& operator=(const file_path& rhs);
        file_path& operator=(file_path&& rhs);
        
        sub_file_path parent() const;
        sub_file_path ext() const;
        sub_file_path stem() const;
        sub_file_path filename() const;
        sub_file_path version() const;
        sub_file_path exclude_version() const;
        sub_file_path pure_filename() const;
        sub_file_path pop_front(size_t n = 1) const;
        sub_file_path pop_back(size_t n = 1) const;
        
        bool is_absolute() const;
        bool is_network() const;
        bool is_root() const;
        bool has_disk() const;
        bool is_pure_disk() const;
        bool is_normalized() const;
        bool empty() const;
        bool has_version() const;
        
        bool under(sub_file_path path) const;
        bool contains(sub_file_path path) const;
        
        file_path& normalize(path_process pp = pp_default);
        
        file_path& operator/=(const sub_file_path& rhs);
        file_path& replace_parent(const file_path& rhs);
        file_path& replace_ext(const file_path& rhs);
        file_path& replace_stem(const file_path& rhs);
        file_path& replace_filename(const file_path& rhs);
        file_path& replace_pure_filename(const file_path& rhs);
        file_path& to_absolute();
        file_path& remove_absolute();
        file_path& remove_version();
        file_path& remove_front(size_t n = 1);
        file_path& remove_back(size_t n = 1);
        file_path& replace_version(sub_file_path version);

		bool replace_prefix(sub_file_path old, sub_file_path new_);
		bool replace_postfix(sub_file_path old, sub_file_path new_);
        
        void swap(file_path& rhs);
        
        const string& str() const;
        
        string native_str() const;
        wstring native_wstr() const;
        wstring wstr() const;
        
        operator sub_file_path() const;
        sub_file_path as_sub_path() const;
        
        iterator begin() const;
        iterator end() const;
        
    private:
        string m_str;
    };
    
    inline bool operator<(const file_path& lhs, const file_path& rhs){
        return lhs.str() < rhs.str();
    }
    inline bool operator<(sub_file_path lhs, const file_path& rhs){
        return lhs.str() < rhs.str();
    }
    inline bool operator<(const file_path& lhs, sub_file_path rhs){
        return lhs.str() < rhs.str();
    }
    inline bool operator==(const file_path& lhs, const file_path& rhs){
        return lhs.str() == rhs.str();
    }
    inline bool operator==(sub_file_path lhs, const file_path& rhs){
        return lhs.str() == rhs.str();
    }
    inline bool operator==(const file_path& lhs, sub_file_path rhs){
        return lhs.str() == rhs.str();
    }
    
    XR_API file_path operator/(const sub_file_path& lhs, const sub_file_path& rhs);
    
    
    class XR_API  sub_simple_path
    {
    public:
        static const char dim;
        sub_simple_path();
        explicit sub_simple_path(const_range_string str);
        explicit sub_simple_path(string::const_iterator first,
                                 string::const_iterator last);
        sub_simple_path& operator=(const sub_simple_path& rhs);
        void swap(sub_simple_path& rhs);
        
        sub_simple_path parent() const;
        sub_simple_path filename() const;
        bool is_absolute() const;
        bool is_root() const;
        bool is_normalized() const;
        bool empty() const;
        
        bool under(sub_simple_path path) const;
        bool contains(sub_simple_path path) const;
        
        const_range_string str() const;
        
        class iterator;
        typedef iterator const_iterator;
        
        iterator begin() const;
        iterator end() const;
    private:
        const_range_string m_str;
    };
    
    inline bool operator<(const sub_simple_path& lhs, const sub_simple_path& rhs){
        return lhs.str() < rhs.str();
    }
    inline bool operator==(const sub_simple_path& lhs, const sub_simple_path& rhs){
        return lhs.str() == rhs.str();
    }
    class sub_simple_path_compare_ : totally_ordered<sub_simple_path>{};
    
    class XR_API sub_simple_path::iterator{
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef sub_simple_path 	value_type;
        typedef std::ptrdiff_t	difference_type;
        typedef const sub_simple_path*	pointer;
        typedef const sub_simple_path&	reference;
        
        iterator();
        void swap(iterator& rhs);
        
        iterator& operator++();
        iterator& operator--();
        iterator operator++(int);
        iterator operator--(int);
        
        bool operator==(const iterator& rhs);
        bool operator!=(const iterator& rhs);
        
        reference operator*() const;
        pointer operator->() const;
        
    private:
        string::const_iterator begin_() const;
        string::const_iterator end_() const;
        sub_simple_path path_() const;
        iterator(const_range_string path, string::const_iterator pos);
        
        friend class sub_simple_path;
        string::const_iterator m_pos;
        const_range_string m_path;
        mutable sub_simple_path m_cache;
    };
    
    class XR_API simple_path : totally_ordered<simple_path>
    {
    public:
        typedef sub_simple_path::iterator iterator;
        typedef sub_simple_path::const_iterator const_iterator;
        simple_path();
        explicit simple_path(const string& str, path_process pp = pp_default);
        simple_path(sub_simple_path rhs);
        simple_path(const simple_path& rhs);
        simple_path(simple_path&& rhs);
        
        simple_path& operator=(const simple_path& rhs);
        simple_path& operator=(simple_path&& rhs);
        simple_path& operator=(sub_simple_path rhs);
        
        operator sub_simple_path() const;
        sub_simple_path as_sub_path() const;
        
        sub_simple_path parent() const;
        sub_simple_path filename() const;
        bool is_absolute() const;
        bool is_root() const;
        bool is_normalized() const;
        bool empty() const;
        bool under(sub_simple_path path) const;
        bool contains(sub_simple_path path) const;
        
        simple_path& normalize(path_process pp = pp_default);
        simple_path& operator/=(const sub_simple_path& rhs);
        simple_path& replace_parent(const simple_path& rhs);
        simple_path& replace_filename(const simple_path& rhs);
        simple_path& to_absolute();
        simple_path& remove_absolute();
        
        void swap(simple_path& rhs);
        
        const string& str() const;
        
        iterator begin() const;
        iterator end() const;
        
    private:
        string m_str;
    };
    
    inline bool operator<(const simple_path& lhs, const simple_path& rhs){
        return lhs.str() < rhs.str();
    }
    inline bool operator<(sub_simple_path lhs, const simple_path& rhs){
        return lhs.str() < rhs.str();
    }
    inline bool operator<(const simple_path& lhs, sub_simple_path rhs){
        return lhs.str() < rhs.str();
    }
    inline bool operator==(const simple_path& lhs, const simple_path& rhs){
        return lhs.str() == rhs.str();
    }
    inline bool operator==(sub_simple_path lhs, const simple_path& rhs){
        return lhs.str() == rhs.str();
    }
    inline bool operator==(const simple_path& lhs, sub_simple_path rhs){
        return lhs.str() == rhs.str();
    }
    
    XR_API simple_path operator/(const sub_simple_path& lhs, const sub_simple_path& rhs);
    
    template<typename PathType>
    bool starts_with(PathType&& lhs, PathType&& rhs){
        auto itr1(lhs.str().begin()), end1(lhs.str().end());
        auto itr2(rhs.str().begin()), end2(rhs.str().end());
        for (; itr1 != end1 && itr2 != end2; ++itr1, ++itr2){
            if (*itr1 != *itr2) return false;
        }
        return itr2 == end2;
    }
    template<typename PathType>
    bool ends_with(PathType&& lhs, PathType&& rhs){
        auto itr1(make_reverse_iterator(lhs.str().end())), end1(make_reverse_iterator(lhs.str().begin()));
        auto itr2(make_reverse_iterator(rhs.str().end())), end2(make_reverse_iterator(rhs.str().begin()));
        for (; itr1 != end1 && itr2 != end2; ++itr1, ++itr2){
            if (*itr1 != *itr2) return false;
        }
        return itr2 == end2;
    }
    
    struct path_less
    {
        template<typename PathType>
        bool less_str(const PathType& lhs, const PathType& rhs) const{
            auto itr1(lhs.str().begin()), end1(lhs.str().end());
            auto itr2(rhs.str().begin()), end2(rhs.str().end());
            unsigned int dim (sub_file_path::dim);
            for (; itr1 != end1 && itr2 != end2; ++itr1, ++itr2){
                unsigned int ch1 = *itr1;
                if (ch1 == dim) ch1 = 0;
                unsigned int ch2 = *itr2;
                if (ch2 == dim) ch2 = 0;
                if (ch1 < ch2) return true;
                if (ch2 < ch1) return false;
            }
            return itr1 == end1 && itr2 != end2;
        }
        template<typename PathType>
        bool operator()(const PathType& lhs, const PathType& rhs) const{
            auto lp = lhs.parent();
            auto rp = rhs.parent();
            return less_str(lp, rp)
            || (lp == rp && less_str(lhs, rhs));
        }
    };
    struct hash_file_path{
        template<typename PathType>
        size_t operator()(const PathType& path) const{
            return path.str().hash();
        }
    };
    struct uri_info
    {
        string scheme;
        string site;
        string port;
        file_path path;
    };
    
    XR_API extern uri_info parse_uri(const file_path& uri);
}

#endif //end XR_COMMON_PATH_H__
