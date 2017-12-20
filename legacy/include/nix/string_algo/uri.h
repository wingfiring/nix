#ifndef XR_COMMON_STRING_ALGO_URI_H__
#define XR_COMMON_STRING_ALGO_URI_H__

#include <xirang2/string.h>
#include <xirang2/context_except.h>

//STL

namespace xirang2 { namespace uri {
    
    XR_EXCEPTION_TYPE(invalid_encoded_uri);
    XR_EXCEPTION_TYPE(no_enough_encoded_uri_input);
    
    template<typename InputIter, typename OutputIter>
    std::pair<InputIter, OutputIter> encode(range<InputIter> in, OutputIter iout)
    {
        InputIter iin = in.begin();
        
        typedef typename std::iterator_traits<InputIter>::value_type  in_value_type_;
        typedef typename std::make_unsigned<in_value_type_>::type in_value_type;
        
        for (; iin != in.end(); ++iin)
        {
            in_value_type ch_in =(*iin & 0xff);
            
            switch (ch_in)
            {
                case '-':
                case '_':
                case '.':
                case '~':
                    *iout = ch_in;
                    break;
                default:
                    if ( (ch_in >= 'A' && ch_in <= 'Z')
                        ||(ch_in >= 'a' && ch_in <= 'z')
                        || (ch_in >= '0' && ch_in <= '9'))
                    {
                        *iout = ch_in;
                    }
                    else
                    {
                        static const char hex_table[] = "0123456789ABCDEF";
                        
                        *iout = '%';
                        *++iout = hex_table[ch_in >> 4];
                        *++iout = hex_table[ch_in & 0x0f];
                    }
            }
            ++iout;
        }
        
        return std::make_pair(iin, iout);
    }
    
    template<typename InputIter, typename OutputIter>
    std::pair<InputIter, OutputIter> http_encode(range<InputIter> in, OutputIter iout)
    {
        InputIter iin = in.begin();
        
        typedef typename std::iterator_traits<InputIter>::value_type  in_value_type_;
        typedef typename std::make_unsigned<in_value_type_>::type in_value_type;
        
        for (; iin != in.end(); ++iin)
        {
            in_value_type ch_in =(*iin & 0xff);
            
            switch (ch_in)
            {
                case '-':
                case '_':		// unreserved
                case '.':
                case '!':
                case '~':
                case '*':
                case '\'':
                case '(':
                case ')':
                    *iout = ch_in;
                    break;
                case ' ':
                    *iout = '+';
                    break;
                default:
                    if ( (ch_in >= 'A' && ch_in <= 'Z')
                        ||(ch_in >= 'a' && ch_in <= 'z')
                        || (ch_in >= '0' && ch_in <= '9'))
                    {
                        *iout = ch_in;
                    }
                    else
                    {
                        static const char hex_table[] = "0123456789ABCDEF";
                        
                        *iout = '%';
                        *++iout = hex_table[ch_in >> 4];
                        *++iout = hex_table[ch_in & 0x0f];
                    }
            }
            ++iout;
        }
        
        return std::make_pair(iin, iout);
    }
    
    namespace private_
    {
        inline void check_throw(bool need_throw)
        {
            if (need_throw)
                XR_THROW(no_enough_encoded_uri_input);
        }
        
        inline int char_hex_to_int(int ch)
        {
            if (ch >= '0' && ch <= '9')
            {
                return (ch - '0');
            }
            else if (ch >= 'A' && ch <= 'F')
            {
                return (ch - 'A' + 10);
            }
            else if (ch >= 'a' && ch <= 'f')
            {
                return (ch - 'a' + 10);
            }
            else
                XR_THROW(invalid_encoded_uri);
        }
    }
    
    template<typename InputIter, typename OutputIter>
    std::pair<InputIter, OutputIter> decode(range<InputIter> in, OutputIter iout)
    {
        InputIter iin = in.begin();
        InputIter iend = in.end();
        
        for (; iin != iend ; ++iin, ++iout)
        {
            typedef typename std::iterator_traits<InputIter>::value_type  in_value_type;
            in_value_type ch = *iin;
            if (ch == '%')
            {
                private_::check_throw (++iin == iend);
                in_value_type tmp = in_value_type(private_::char_hex_to_int(*iin) << 4);
                
                private_::check_throw (++iin == iend);
                tmp |= private_::char_hex_to_int(*iin);
                
                ch = tmp;
            }
            
            *iout = ch;
        }
        return std::make_pair(iin, iout);
    }
    
    template<typename InputIter, typename OutputIter>
    std::pair<InputIter, OutputIter> http_decode(range<InputIter> in, OutputIter iout)
    {
        InputIter iin = in.begin();
        InputIter iend = in.end();
        
        for (; iin != iend ; ++iin, ++iout)
        {
            typedef typename std::iterator_traits<InputIter>::value_type  in_value_type;
            in_value_type ch = *iin;
            
            if (ch == '%')
            {
                private_::check_throw (++iin == iend);
                in_value_type tmp = in_value_type(private_::char_hex_to_int(*iin) << 4);
                
                private_::check_throw (++iin == iend);
                tmp |= private_::char_hex_to_int(*iin);
                
                ch = tmp;
            }
            else if(ch == '+'){
                ch = ' ';
            }
            
            *iout = ch;
        }
        return std::make_pair(iin, iout);
    }
    
    
    
    template<typename Cont, typename SrcCont>
    Cont encode_to(const SrcCont& rhs)
    {
        Cont sb;
        encode(to_range(rhs), std::back_inserter(sb));
        return sb;
    }
    
    template<typename Cont, typename SrcCont>
    Cont decode_to(const SrcCont& rhs)
    {
        Cont sb;
        decode(to_range(rhs), std::back_inserter(sb));
        return sb;
    }
    
    template<typename Cont, typename SrcCont>
    Cont http_encode_to(const SrcCont& rhs)
    {
        Cont sb;
        http_encode(to_range(rhs), std::back_inserter(sb));
        return sb;
    }
    
    template<typename Cont, typename SrcCont>
    Cont http_decode_to(const SrcCont& rhs)
    {
        Cont sb;
        http_decode(to_range(rhs), std::back_inserter(sb));
        return sb;
    }
    
    template<typename SrcCont>
    string encode_string(const SrcCont& rhs)
    {
        string_builder sb;
        encode(to_range(rhs), std::back_inserter(sb));
        return string(sb);
    }
    
    template<typename SrcCont>
    string decode_string(const SrcCont& rhs)
    {
        string_builder sb;
        decode(to_range(rhs), std::back_inserter(sb));
        return string(sb);
    }
    
    template<typename SrcCont>
    string http_encode_string(const SrcCont& rhs)
    {
        string_builder sb;
        http_encode(to_range(rhs), std::back_inserter(sb));
        return string(sb);
    }
    
    template<typename SrcCont>
    string http_decode_string(const SrcCont& rhs)
    {
        string_builder sb;
        http_decode(to_range(rhs), std::back_inserter(sb));
        return string(sb);
    }
    
    struct uri_info
    {
        string scheme;
        string site;
        string port;
        string path;
    };
    
}}

#endif //end XR_COMMON_STRING_ALGO_URI_H__

