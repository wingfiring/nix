#include <xirang2/path.h>
#include <algorithm>
#include <vector>
#include <xirang2/string_algo/string.h>
#include <xirang2/sha1.h>

namespace xirang2{
    namespace{
        const auto onedot = literal(".");
        const auto twodot = literal("..");
    }

	static void check_utf8_(const string& str) {
		utf8::decode_string(str);
	}
	const char sub_file_path::dim = '/';

	sub_file_path::sub_file_path(){}

	sub_file_path::sub_file_path(const_range_string str)
		: m_str(str)
	{ }
	sub_file_path::sub_file_path(string::const_iterator first,
			string::const_iterator last)
		: m_str(first, last)
	{}

	sub_file_path& sub_file_path::operator=(const sub_file_path& rhs){
		m_str = rhs.m_str;
		return *this;
	}
	void sub_file_path::swap(sub_file_path& rhs){
		std::swap(m_str, rhs.m_str);
	}

	sub_file_path sub_file_path::parent() const{
		auto pos = rfind(m_str, dim);
		if ((pos ==  m_str.begin() && is_absolute())
			|| (is_network() && pos == m_str.begin() + 1)){
			return *begin();
		}
		if (pos == m_str.end()) pos = m_str.begin();
		return sub_file_path(m_str.begin(), pos);
	}

	sub_file_path sub_file_path::ext() const{
		auto pos_dim = rfind(m_str, dim);
        if (pos_dim == m_str.end())
            pos_dim = m_str.begin();
		auto pos = rfind(pos_dim, m_str.end(), '.');
		if (pos != m_str.end()) ++pos;
        
        auto sharp_pos = rfind(pos, m_str.end(), '#');
        if (sharp_pos == m_str.end()){
            sharp_pos = m_str.end();
        }
		return sub_file_path(pos, sharp_pos);
	}
    
	sub_file_path sub_file_path::stem() const{
		auto pos1 = rfind(m_str, dim);
		if (pos1 != m_str.end()) ++pos1;
		auto pos2 = rfind(pos1, m_str.end(), '.');
		return sub_file_path(pos1, pos2);
	}
    
	sub_file_path sub_file_path::filename() const{
		auto pos = rfind(m_str, dim);
		if (pos == m_str.end()) 
			pos = m_str.begin();
		else
			++pos;
		        
        return sub_file_path(pos, m_str.end());
	}

    sub_file_path sub_file_path::version() const{
        auto pos = rfind(m_str, dim);
        if (pos == m_str.end())
            pos = m_str.begin();
        else
            ++pos;
        
        auto sharp_pos = rfind(pos, m_str.end(), '#');
        if (sharp_pos != m_str.end())
            ++sharp_pos;
        
		const_range_string possible_version_str(sharp_pos, m_str.end());
		bool is_version = is_valid_sha1_str(possible_version_str);
        return is_version ? sub_file_path(possible_version_str) : sub_file_path();
    }
    
    sub_file_path sub_file_path::exclude_version() const{
		auto version_part = version();
		auto sharp_pos = version_part.empty() ? m_str.end() : version_part.str().begin() - 1;

        return sub_file_path(m_str.begin(), sharp_pos);
    }
    
    sub_file_path sub_file_path::pure_filename() const{
        auto pos = rfind(m_str, dim);
        if (pos == m_str.end())
            pos = m_str.begin();
        else
            ++pos;
        
        auto sharp_pos = rfind(pos, m_str.end(), '#');
        return sub_file_path(pos, sharp_pos);
    }
	sub_file_path sub_file_path::pop_front(size_t n) const{
		auto itr = begin();
		while (n-- !=0 && itr != end())
			++itr;
		return itr == end() ? sub_file_path() : sub_file_path(const_range_string(itr->str().begin(), m_str.end()));
	}
	sub_file_path sub_file_path::pop_back(size_t n) const{
		sub_file_path ret(*this);
		while(n-- != 0)
			ret = ret.parent();
		return ret;
	}
	bool sub_file_path::is_absolute() const{
		return !m_str.empty() && m_str[0] == dim;
	}
	bool sub_file_path::is_network() const{
		return m_str.size() > 2
			&& m_str[0] == dim 
			&& m_str[1] == dim
			&& m_str[2] != dim;
	}
	bool sub_file_path::is_root() const{
		return m_str.size() == 1 && m_str[0] == dim;
	}
	bool sub_file_path::has_disk() const{
		return m_str.size() > 2
			&& m_str[2] == ':';
	}
	bool sub_file_path::is_pure_disk() const{
		return m_str.size() == 3
			&& m_str[2] == ':';
	}
	bool sub_file_path::is_normalized() const{
        bool leading_twodot = true;
		return std::none_of(begin(), end(), [&leading_twodot](const sub_file_path& p){
            if (leading_twodot){
                if (p.str() == twodot)
                    return false;
                else leading_twodot = false;
            }
            return p.str().empty() || p.str() == onedot || p.str() == twodot;
            
        });
	}
	bool sub_file_path::empty() const{
		return m_str.empty();
	}
    bool sub_file_path::has_version() const{
        return !version().empty();
    }
	bool sub_file_path::under(sub_file_path path_) const{
        auto path = path_.exclude_version();
        auto self_ = exclude_version();
		if( self_.m_str.size() <= path.str().size())
			return false;
		auto pos = std::mismatch(self_.m_str.begin(), self_.m_str.end(), path.str().begin());
		if (pos.second != path.str().end())
			return false;

		return (path.is_root() && self_.is_absolute()) || *pos.first == dim;
		
	}
	bool sub_file_path::contains(sub_file_path path) const{
		return path.under(*this);
	}

	const_range_string sub_file_path::str() const{
		return m_str;
	}
	string sub_file_path::native_str() const{
#ifdef WIN32_OS_
		auto ret = replace(m_str, dim, char('\\'));
		if (is_absolute() && !is_network()){
			return const_range_string(ret.begin() + 1, ret.end());
		}
		return std::move(ret);
#else
		return str();
#endif
	}
	wstring sub_file_path::native_wstr() const{
		return utf8::decode_string(native_str());
	}
	wstring sub_file_path::wstr() const{
		return utf8::decode_string(m_str);
	}
	sub_file_path::iterator sub_file_path::begin() const{
		return sub_file_path::iterator(m_str, m_str.begin());
	}
	sub_file_path::iterator sub_file_path::end() const{
		return sub_file_path::iterator(m_str, m_str.end());
	}


	sub_file_path::iterator::iterator()
		: m_pos(), m_path()
	{}
	sub_file_path::iterator::iterator(const_range_string spath, string::const_iterator pos)
		: m_pos(pos), m_path(spath)
	{}

	void sub_file_path::iterator::swap(sub_file_path::iterator& rhs){
		std::swap(m_pos, rhs.m_pos);
		std::swap(m_path, rhs.m_path);
	}

	sub_file_path::iterator& sub_file_path::iterator::operator++(){
		XR_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		XR_PRE_CONDITION(m_pos != end_() &&"increase end iterator");

		if (m_pos == begin_()){
			if(path_().is_network()){
				m_pos += 2;
			}
			else if(path_().is_absolute()){
				++m_pos;
				return *this;
			}
		}
		m_pos = find(m_pos, end_(), sub_file_path::dim);
		if (m_pos != end_()) ++m_pos;
		return *this;
	}
	sub_file_path::iterator& sub_file_path::iterator::operator--(){
		XR_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		XR_PRE_CONDITION(m_pos != begin_() &&"decrease begin iterator");

		if (m_pos == begin_() + 1 && path_().is_absolute()){
			--m_pos;
			return *this;
		}

		if (m_pos != begin_() && m_pos != end_()){
			XR_PRE_CONDITION(*(m_pos -1) == sub_file_path::dim);
			--m_pos;
		}
		auto pos = rfind(begin_(), m_pos, sub_file_path::dim);
		m_pos = (pos == m_pos) ? begin_() : pos + 1;

		if (path_().is_network() && m_pos == begin_() + 1)
			m_pos = begin_();
		
		return *this;
	}
	sub_file_path::iterator sub_file_path::iterator::operator++(int){
		auto tmp = *this;
		++*this;
		return tmp;
	}
	sub_file_path::iterator sub_file_path::iterator::operator--(int){
		auto tmp = *this;
		--*this;
		return tmp;
	}

	bool sub_file_path::iterator::operator==(const sub_file_path::iterator& rhs){
		XR_PRE_CONDITION(m_path == rhs.m_path);

		return m_pos == rhs.m_pos;
	}
	bool sub_file_path::iterator::operator!=(const sub_file_path::iterator& rhs){
		return !(*this == rhs);
	}

	sub_file_path::iterator::reference sub_file_path::iterator::operator*() const{
		XR_PRE_CONDITION(m_pos != end_() &&"dereference end iterator");
		auto pos = m_pos;
		if (m_pos == begin_()){
			if(path_().is_network())
				pos = find(m_pos + 2, end_(), sub_file_path::dim);
			else if (path_().is_absolute())
				++pos;
			else
				pos = find(m_pos, end_(), sub_file_path::dim);
		}
		else
			pos = find(m_pos, end_(), sub_file_path::dim);

		m_cache = sub_file_path(m_pos, pos);
		return m_cache;
	}
	sub_file_path::iterator::pointer sub_file_path::iterator::operator->() const{
		return &**this;
	}
	sub_file_path sub_file_path::iterator::path_() const{
		return sub_file_path(begin_(), end_());
	}
	string::const_iterator sub_file_path::iterator::begin_() const{
		return m_path.begin();
	}
	string::const_iterator sub_file_path::iterator::end_() const{
		return m_path.end();
	}

	/////////////////////////////////// 
	// file_path

	file_path::file_path() {}
	file_path::file_path(const sub_file_path& spath) 
		: m_str(spath.str())
	{}

	file_path::file_path(const string& str, path_process pp /* = pp_default*/)
		: m_str(str)
	{
        normalize(pp);
	}
	file_path::file_path(const wstring& str, path_process pp /* = pp_default */)
		: m_str(utf8::encode_string(str))
	{
        normalize(pp);
	}
	file_path::file_path(const file_path& rhs) : m_str(rhs.m_str){}
	file_path::file_path(file_path&& rhs) : m_str(std::move(rhs.m_str)){}

	file_path& file_path::operator=(const file_path& rhs){
		file_path(rhs).swap(*this);
		return *this;
	}
	file_path& file_path::operator=(file_path&& rhs){
		file_path(std::move(rhs)).swap(*this);
		return *this;
	}


	sub_file_path file_path::parent() const{
		return as_sub_path().parent(); 
	}
	sub_file_path file_path::ext() const{
		return as_sub_path().ext();
	}
	sub_file_path file_path::stem() const{
		return as_sub_path().stem();
	}
	sub_file_path file_path::filename() const{
		return as_sub_path().filename();
	}
    sub_file_path file_path::version() const{
        return  as_sub_path().version();
    }
    sub_file_path file_path::exclude_version() const{
        return as_sub_path().exclude_version();
    }
    sub_file_path file_path::pure_filename() const {
        return as_sub_path().pure_filename();
    }
	sub_file_path file_path::pop_front(size_t n) const{
		return as_sub_path().pop_front(n);
	}
	sub_file_path file_path::pop_back(size_t n) const{
		return as_sub_path().pop_back(n);
	}
    

	bool file_path::is_absolute() const{
		return as_sub_path().is_absolute();
	}
	bool file_path::is_network() const{
		return as_sub_path().is_network();
	}
	bool file_path::is_root() const{
		return as_sub_path().is_root();
	}
	bool file_path::has_disk() const{
		return as_sub_path().has_disk();
	}
	bool file_path::is_pure_disk() const{
		return as_sub_path().is_pure_disk();
	}

	bool file_path::is_normalized() const{
		return as_sub_path().is_normalized();
	}
	bool file_path::empty() const{
		return m_str.empty();
	}
    bool file_path::has_version() const{
        return as_sub_path().has_version();
    }
    
	bool file_path::under(sub_file_path path) const{
		return as_sub_path().under(path);
	}
	bool file_path::contains(sub_file_path path) const{
		return as_sub_path().contains(path);
	}

    file_path& file_path::normalize(path_process pp)
	{
        if (pp & pp_utf8check)
            check_utf8_(m_str);

        if ((pp & pp_convert_backslash) != 0)
            m_str = replace(m_str, '\\', sub_file_path::dim);

        string_builder result;

        if ((pp & pp_normalize) != 0) {
            std::vector<sub_file_path> stack;
            for (auto p : *this) {
                if (p.str() == onedot || p.str().empty())
                    continue;
                if (p.str() != twodot) {
                    stack.push_back(p);
                    continue;
                }
                if (is_absolute() && stack.size() == 1)
                    continue;

                if (!stack.empty() && stack.back().str() != twodot)
                    stack.pop_back();
                else
                    stack.push_back(p);
            }
            for (auto& p : stack)
            {
                result += p.str();
                if (!p.is_root())
                    result.push_back(sub_file_path::dim);
            }
            if (result.size() > 1)
                result.resize(result.size() - 1);
        }
        else
            result = m_str;
		if ((pp & pp_winfile) && result.size() > 1 && result[0] != sub_file_path::dim && result[1] == ':')
			result.insert(result.begin(), 1, sub_file_path::dim);
		m_str = result;
		return *this;
	}


	file_path& file_path::operator/=(const sub_file_path& rhs){
		if (rhs.is_root() || rhs.str().empty()){
			if (empty()) m_str = rhs.str();
			return *this;
		}
		if (m_str.empty()){
			m_str = rhs.str();
			return *this;
		}

		if (is_root())
		{
			if (rhs.is_absolute())
				m_str = rhs.str();
			else
				m_str = literal("/") << rhs.str();
		}
		else if (rhs.is_network()){
			m_str = m_str << rhs.str();
			normalize();
		}
		else if (rhs.is_absolute())
			m_str = m_str << rhs.str();
		else
			m_str = m_str << literal("/") << rhs.str();
		return *this;
	}

	file_path& file_path::replace_parent(const file_path& rhs){
		*this = rhs / filename();
		return *this;
	}
	file_path& file_path::replace_ext(const file_path& rhs){
        auto pos_dim = rfind(m_str, sub_file_path::dim);
        if (pos_dim == m_str.end())
            pos_dim = m_str.begin();
		auto pos = rfind(pos_dim, m_str.end(), '.');
		if (!rhs.m_str.empty() && rhs.m_str[0] == '.')
			m_str = const_range_string(m_str.begin(), pos) << rhs.m_str;
		else 
			m_str = const_range_string(m_str.begin(), pos) << literal(".") << rhs.m_str;

		return *this;
	}
	file_path& file_path::replace_stem(const file_path& rhs){
		auto extension = ext().str();
		if (extension.empty())
			*this = parent() / rhs;
		else
			*this = parent() / file_path(string(rhs.str() << literal(".") <<  extension), pp_none);

		return *this;
	}
	file_path& file_path::replace_filename(const file_path& rhs){
        return *this = parent() / rhs;
	}
    file_path& file_path::replace_pure_filename(const file_path& rhs){
        auto ver = version();
        if (ver.empty()){
            *this = parent() / rhs;
        }
        else {
            this->m_str = (parent() / rhs).str() << literal("#") << ver.str();
        }
        return *this;
    }
	file_path& file_path::to_absolute(){
		if (!is_absolute())
			m_str = literal("/") << m_str;
		return *this;
	}
	file_path& file_path::remove_absolute(){
		if (is_network())
			m_str = const_range_string(m_str.begin() + 2, m_str.end());
		else if(is_absolute()){
			m_str = const_range_string(m_str.begin() + 1, m_str.end());
		}
		return *this;
	}

    file_path& file_path::remove_version(){
        if (has_version())
            *this = exclude_version();
        return *this;
    }
	file_path& file_path::remove_front(size_t n){
		if (n != 0)
			*this = pop_front(n);
		return *this;
	}
	file_path& file_path::remove_back(size_t n){
		if (n != 0)
			*this = pop_back(n);
		return *this;
	}
    
    file_path& file_path::replace_version(sub_file_path version){
		XR_PRE_CONDITION(is_valid_sha1_str(version.str()));
        m_str = exclude_version().str() << literal("#") << version.str();
        return *this;
    }
    
	bool file_path::replace_prefix(sub_file_path oldp, sub_file_path newp){
		auto old = oldp.str();
		auto new_ = newp.str();

		if (m_str.size() < old.size()) 
			return false;

		if (!std::equal(old.begin(), old.end(), m_str.begin()))
			return false;

		m_str = (new_ << const_range_string(m_str.begin() + old.size(), m_str.end()));

		return true;
	}

	bool file_path::replace_postfix(sub_file_path oldp, sub_file_path newp){
		auto old = oldp.str();
		auto new_ = newp.str();

		if (m_str.size() < old.size()) 
			return false;

		if (!std::equal(m_str.begin() + (m_str.size() - old.size()), m_str.end(), old.begin()))
			return false;
		m_str = (const_range_string(m_str.begin(), m_str.begin()  + (m_str.size() - old.size())) << new_);
		return true;
	}

	void file_path::swap(file_path& rhs){
		m_str.swap(rhs.m_str);
	}

	const string& file_path::str() const{
		return m_str;
	}

	string file_path::native_str() const{
		return as_sub_path().native_str();
	}
	wstring file_path::native_wstr() const{
		return as_sub_path().native_wstr();
	}
	wstring file_path::wstr() const{
		return as_sub_path().wstr();
	}

	file_path::operator sub_file_path() const{
		return as_sub_path();
	}
	sub_file_path file_path::as_sub_path() const{
		return sub_file_path(m_str.begin(), m_str.end());
	}
	file_path::iterator file_path::begin() const{
		return as_sub_path().begin();
	}
	file_path::iterator file_path::end() const{
		return as_sub_path().end();
	}

	file_path operator/(const sub_file_path& lhs, const sub_file_path& rhs){
		file_path ret = lhs;
		ret /= rhs;
		return ret;
	}

	////////////////////////////////
	//sub_simple_path

	const char sub_simple_path::dim = '.';
	sub_simple_path::sub_simple_path(){}

	sub_simple_path::sub_simple_path(const_range_string str)
		: m_str(str)
	{ }
	sub_simple_path::sub_simple_path(string::const_iterator first,
			string::const_iterator last)
		: m_str(first, last)
	{}

	sub_simple_path& sub_simple_path::operator=(const sub_simple_path& rhs){
		m_str = rhs.m_str;
		return *this;
	}
	void sub_simple_path::swap(sub_simple_path& rhs){
		std::swap(m_str, rhs.m_str);
	}

	sub_simple_path sub_simple_path::parent() const{
		auto pos = rfind(m_str, dim);
		if (pos ==  m_str.begin() && is_absolute()){
			return *begin();
		}
		if (pos == m_str.end()) pos = m_str.begin();
		return sub_simple_path(m_str.begin(), pos);
	}
	sub_simple_path sub_simple_path::filename() const{
		auto pos = rfind(m_str, dim);
		if (pos == m_str.end()) 
			pos = m_str.begin();
		else
			++pos;
		return sub_simple_path(pos, m_str.end());
	}

	bool sub_simple_path::is_absolute() const{
		return !m_str.empty() && m_str[0] == dim;
	}
	bool sub_simple_path::is_root() const{
		return m_str.size() == 1 && m_str[0] == dim;
	}
	bool sub_simple_path::is_normalized() const{
		return std::none_of(begin(), end(), [](const sub_simple_path& p){ return p.str().empty();});
	}
	bool sub_simple_path::empty() const{
		return m_str.empty();
	}

	bool sub_simple_path::under(sub_simple_path path) const{
		if( m_str.size() <= path.str().size())
			return false;
		auto pos = std::mismatch(m_str.begin(), m_str.end(), path.str().begin());
		if (pos.second != path.str().end())
			return false;
		return *pos.first == dim;
		
	}
	bool sub_simple_path::contains(sub_simple_path path) const{
		return path.under(*this);
	}

	const_range_string sub_simple_path::str() const{
		return m_str;
	}
	sub_simple_path::iterator sub_simple_path::begin() const{
		return sub_simple_path::iterator(m_str, m_str.begin());
	}
	sub_simple_path::iterator sub_simple_path::end() const{
		return sub_simple_path::iterator(m_str, m_str.end());
	}

	sub_simple_path::iterator::iterator()
		: m_pos(), m_path()
	{}
	sub_simple_path::iterator::iterator(const_range_string spath, string::const_iterator pos)
		: m_pos(pos), m_path(spath)
	{}

	void sub_simple_path::iterator::swap(sub_simple_path::iterator& rhs){
		std::swap(m_pos, rhs.m_pos);
		std::swap(m_path, rhs.m_path);
	}

	sub_simple_path::iterator& sub_simple_path::iterator::operator++(){
		XR_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		XR_PRE_CONDITION(m_pos != end_() &&"increase end iterator");

		if (m_pos == begin_() &&path_().is_absolute()){
			++m_pos;
			return *this;
		}
		m_pos = find(m_pos, end_(), sub_simple_path::dim);
		if (m_pos != end_()) ++m_pos;
		return *this;
	}
	sub_simple_path::iterator& sub_simple_path::iterator::operator--(){
		XR_PRE_CONDITION(!m_path.empty() && m_pos && "empty iterator");
		XR_PRE_CONDITION(m_pos != begin_() &&"decrease begin iterator");

		if (m_pos == begin_() + 1 && path_().is_absolute()){
			--m_pos;
			return *this;
		}

		if (m_pos != begin_() && m_pos != end_()){
			XR_PRE_CONDITION(*(m_pos -1) == sub_simple_path::dim);
			--m_pos;
		}
		auto pos = rfind(begin_(), m_pos, sub_simple_path::dim);
		m_pos = (pos == m_pos) ? begin_() : pos + 1;

		return *this;
	}
	sub_simple_path::iterator sub_simple_path::iterator::operator++(int){
		auto tmp = *this;
		++*this;
		return tmp;
	}
	sub_simple_path::iterator sub_simple_path::iterator::operator--(int){
		auto tmp = *this;
		--*this;
		return tmp;
	}

	bool sub_simple_path::iterator::operator==(const sub_simple_path::iterator& rhs){
		XR_PRE_CONDITION(m_path == rhs.m_path);

		return m_pos == rhs.m_pos;
	}
	bool sub_simple_path::iterator::operator!=(const sub_simple_path::iterator& rhs){
		return !(*this == rhs);
	}

	sub_simple_path::iterator::reference sub_simple_path::iterator::operator*() const{
		XR_PRE_CONDITION(m_pos != end_() &&"dereference end iterator");
		auto pos = m_pos;
		if (m_pos == begin_()){
			if (path_().is_absolute())
				++pos;
			else
				pos = find(m_pos, end_(), sub_simple_path::dim);
		}
		else
			pos = find(m_pos, end_(), sub_simple_path::dim);

		m_cache = sub_simple_path(m_pos, pos);
		return m_cache;
	}
	sub_simple_path::iterator::pointer sub_simple_path::iterator::operator->() const{
		return &**this;
	}
	sub_simple_path sub_simple_path::iterator::path_() const{
		return sub_simple_path(begin_(), end_());
	}
	string::const_iterator sub_simple_path::iterator::begin_() const{
		return m_path.begin();
	}
	string::const_iterator sub_simple_path::iterator::end_() const{
		return m_path.end();
	}

	simple_path::simple_path() {}

	simple_path::simple_path(const string& str, path_process pp /* = pp_default*/)
		: m_str(str)
	{
		normalize(pp);
	}
	simple_path::simple_path(const simple_path& rhs) : m_str(rhs.m_str){}
	simple_path::simple_path(sub_simple_path rhs) : m_str(rhs.str()){}
	simple_path::simple_path(simple_path&& rhs) : m_str(std::move(rhs.m_str)){}

	simple_path& simple_path::operator=(const simple_path& rhs){
		simple_path(rhs).swap(*this);
		return *this;
	}
	simple_path& simple_path::operator=(simple_path&& rhs){
		simple_path(std::move(rhs)).swap(*this);
		return *this;
	}
	simple_path& simple_path::operator=(sub_simple_path rhs){
		m_str = rhs.str();
		return *this;
	}
	simple_path::operator sub_simple_path() const{
		return as_sub_path();
	}
	sub_simple_path simple_path::as_sub_path() const{
		return sub_simple_path(m_str.begin(), m_str.end());
	}

	sub_simple_path simple_path::parent() const{
		return as_sub_path().parent();
	}
	sub_simple_path simple_path::filename() const{
		return as_sub_path().filename();
	}
	bool simple_path::is_absolute() const{
		return as_sub_path().is_absolute();
	}
	bool simple_path::is_root() const{
		return as_sub_path().is_root();
	}
	bool simple_path::is_normalized() const{
		return as_sub_path().is_normalized();
	}

	bool simple_path::empty() const{
		return as_sub_path().empty();
	}
	bool simple_path::under(sub_simple_path path) const{
		return as_sub_path().under(path);
	}
	bool simple_path::contains(sub_simple_path path) const{
		return as_sub_path().contains(path);
	}
    simple_path& simple_path::normalize(path_process pp)
	{
        if (pp & pp_utf8check)
            check_utf8_(m_str);
        if (pp & pp_normalize) {
            std::vector<sub_simple_path> stack;
            for (auto p : *this) {
                if (p.str().empty())
                    continue;
                stack.push_back(p);
            }

            string_builder result;
            for (auto& p : stack)
            {
                result += p.str();
                if (!p.is_root())
                    result.push_back(sub_simple_path::dim);
            }
            if (result.size() > 1)
                result.resize(result.size() - 1);
            m_str = result;
        }
		return *this;
	}

	simple_path& simple_path::operator/=(const sub_simple_path& rhs){
        if (rhs.is_root() || rhs.str().empty()){
            if (empty()) m_str = rhs.str();
            return *this;
        }			

		if (m_str.empty()){
			m_str = rhs.str();
			return *this;
		}			

		if (is_root())
		{
			if (rhs.is_absolute())
				m_str = rhs.str();
			else
				m_str = literal(".") << rhs.str();
		}
		else if (rhs.is_absolute())
			m_str = m_str << rhs.str();
		else
			m_str = m_str << literal(".") << rhs.str();
		return *this;
	}

	simple_path& simple_path::replace_parent(const simple_path& rhs){
		*this = rhs / filename();
		return *this;
	}
	simple_path& simple_path::replace_filename(const simple_path& rhs){
		*this = parent() / rhs;
		return *this;
	}
	simple_path& simple_path::to_absolute(){
		if (!is_absolute())
			m_str = literal(".") << m_str;
		return *this;
	}
	simple_path& simple_path::remove_absolute(){
		if(is_absolute()){
			m_str = const_range_string(m_str.begin() + 1, m_str.end());
		}
		return *this;
	}

	void simple_path::swap(simple_path& rhs){
		m_str.swap(rhs.m_str);
	}

	const string& simple_path::str() const{
		return m_str;
	}

	simple_path::iterator simple_path::begin() const{
		return as_sub_path().begin();
	}
	simple_path::iterator simple_path::end() const{
		return as_sub_path().end();
	}

	simple_path operator/(const sub_simple_path& lhs, const sub_simple_path& rhs){
		simple_path ret = lhs;
		ret /= rhs;
		return ret;
	}
    
    //url format: netfs://<host>[:<port>[:]]/<path>
    // if port is number, the second ':' can be ommited
    // if port is pipe or Unix domain socks pipe, the second ':' must be presented
    static const string scheme_dim("://");
    uri_info parse_uri(const file_path& uri_)
    {
        auto& uri = uri_.str();
        uri_info result;
        
        //scheme://host:port/dir/...
        
        auto scheme_end = std::search(uri.begin(), uri.end(), scheme_dim.begin(), scheme_dim.end());
        
        if (scheme_end != uri.end()) {
            result.scheme = string(make_range(uri.begin(), scheme_end));
            scheme_end += scheme_dim.size();
        }
        else
            scheme_end = uri.begin();
        
        // for site
        bool has_port = false;
        auto site_end = std::find(scheme_end, uri.end(), ':');
        if (site_end == uri.end()){
            site_end = std::find(scheme_end, uri.end(), '/');
        }
        else{
            auto site_end1 = std::find(scheme_end, site_end, '/');
            
            has_port = site_end1 == site_end;
            site_end = site_end1;
        }
        result.site = string(make_range(scheme_end, site_end));
        
        //for port
        if (has_port){
            if (site_end != uri.end()) ++site_end;
            auto port_end = std::find(site_end, uri.end(), ':');
            if (port_end == uri.end()){
                port_end = std::find(site_end, uri.end(), '/');
            }
            result.port = string(make_range(site_end, port_end));
            
            site_end = port_end;
        }
        
        result.path = file_path(string(make_range(site_end, uri.end())));
        
        return result;
    }


}

