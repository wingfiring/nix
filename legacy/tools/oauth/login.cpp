#include <xirang2/string.h>
#include <xirang2/context_except.h>
#include <xirang2/io.h>
#include <xirang2/string_algo/string.h>
#include <xirang2/path.h>

#pragma warning( disable : 4005 )
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ssl.hpp>


#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>


#include <iostream>
#include <memory>
#include <map>
#include <sstream>

XR_INTERNAL_EXCEPTION_TYPE(network_exception);


#if 0
namespace http {
    class parameters;
    class parameter_map;
    class parameter_multi_map;
    class parameter_iterator;

    class headers;
    class header_map;
    class header_multi_map;
    class header_iterator;

    class content : public xirang2::io::reader {};

    class connection {
        request talk();
    };

    class request {
        response send();
    };
    class response {
        request complete();
    };


    class session_mgr {

    };

    struct cookie {

    };

    void foo() {
        connection* conn = make_conn(
            io_service,
            url,
            cookies,
            headers,

        );
    }
}

#endif
typedef std::map<std::string, std::string> CookieMap;
typedef std::multimap<std::string, std::string> HeadMap;
struct httpHead {
    std::string version, code, status;
    std::string header_str;
    HeadMap headers;
};

std::multimap<std::string, std::string> load_headers_(std::istream& sstr) {
    std::multimap<std::string, std::string> ret;
    while (sstr)
    {
        std::string name, line;
        sstr >> name;
        getline(sstr, line);
        if (!name.empty()) name.resize(name.size() - 1);

        if (name.empty()) continue;

        boost::trim(line);
        boost::to_lower(name);
        ret.insert({ name, line });
    }
    return ret;
}

void saveCookies(const HeadMap& hmap, CookieMap& cookies) {
    auto range = hmap.equal_range("set-cookie");
    for (auto pos = range.first; pos != range.second; ++pos) {
        std::vector<std::string> cookie;
        boost::split(cookie, pos->second, [](char ch) { return ch == ';'; });
        if (cookie.empty()) continue;

        for (auto itr = ++cookie.begin(); itr != cookie.end(); ++itr)
            if (*itr == "HttpOnly")
                continue;

        auto& cookie_pair = *cookie.begin();
        auto itr = std::find(cookie_pair.begin(), cookie_pair.end(), '=');
        std::string name = std::string(cookie_pair.begin(), itr);
        if (itr != cookie_pair.end()) ++itr;
        std::string value = std::string(itr, cookie_pair.end());
        cookies[name] = value;
    }
}
std::string genCookieHeader(CookieMap& cookies){
    std::string cookie_head("Cookie:");
    for (auto& i : cookies) {
        cookie_head.push_back(' ');
        cookie_head += i.first;
        cookie_head.push_back('=');
        cookie_head += i.second;
        cookie_head.push_back(';');
    }
    if (!cookies.empty()) cookie_head.resize(cookie_head.size() - 1);
    return cookie_head;
}

void connectHttps(const std::string& host, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket,  boost::asio::yield_context& yield, const std::string& port = "443") {
    using boost::asio::ip::tcp;

    boost::system::error_code ec;
    auto& io_service = socket.get_io_service();

    tcp::resolver resolver(io_service);
    auto itr = resolver.async_resolve(tcp::resolver::query(host.c_str(), port), yield[ec]);
    if (ec)
        XR_THROW(network_exception)("Failed to resolve login_site ")(host.c_str());

    for (tcp::resolver::iterator end; itr != end; ++itr) {
        boost::asio::async_connect(socket.lowest_layer(), itr, yield[ec]);
        if (!ec) break;
    }
    if (!socket.lowest_layer().is_open())
        XR_THROW(network_exception)("connect login_site ")(host.c_str());

    socket.async_handshake(boost::asio::ssl::stream_base::client, yield[ec]);
    if (ec) {
        std::cout << "async_handshake:" << ec.message() << std::endl;
        XR_THROW(network_exception)("TLS handshake ")(host.c_str());
    }
}

httpHead httpsSend(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& socket, const std::string& in, std::vector<char>& data, boost::asio::yield_context& yield) {
    boost::system::error_code ec;
    boost::asio::async_write(socket, boost::asio::buffer(in), yield[ec]);
    if (ec)
        XR_THROW(network_exception)("Send request:\n")(in.c_str());

    data.resize(65536);
    std::size_t n = socket.async_read_some(boost::asio::buffer(data), yield[ec]);
    if (ec)
        XR_THROW(network_exception)("Failed to read response");

    data.resize(n);

    static const std::string CRLF_Dim = "\r\n\r\n";
    auto crlfcrlf = std::search(data.begin(), data.end(), CRLF_Dim.begin(), CRLF_Dim.end());
    if (crlfcrlf == data.end())
        XR_THROW(network_exception)("Bad response header");

    httpHead head;
    std::string headers(data.begin(), crlfcrlf);
    std::stringstream sheader(headers);
    
    sheader >> head.version >> head.code;
    std::getline(sheader, head.status);
    std::cout << "response with code " << head.code << std::endl;

    head.headers = load_headers_(sheader);
    head.header_str = headers;
    return head;
}
const xirang2::string login_url("https://accounts-dev.autodesk.com/Authentication/LogOn");

const xirang2::string login_site("accounts-dev.autodesk.com");

const xirang2::string oauth_site("developer-dev.api.autodesk.com");



void test_login() {
    using boost::asio::ip::tcp;

    boost::asio::io_service io_service;
    boost::asio::spawn(io_service,
        [&](boost::asio::yield_context yield)
    {
        boost::system::error_code ec;
        //boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12);
        boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12);
        
        /// Connect
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket(io_service, ssl_context);
        connectHttps(login_site.c_str(), socket, yield);

        //// call Authentication/LogOn:
        std::string scontent ="UserName=jackson.sun%40autodesk.com&Password=jqmthmlsb123";
        std::stringstream sstr;

        /// send logon request
        sstr << "POST /Authentication/LogOn HTTP/1.1""\r\n"
            "Host: accounts-dev.autodesk.com""\r\n"
            "Connection: keep-alive""\r\n"
            "Content-Length : " << scontent.size() << "\r\n"
            "Content-Type: application/x-www-form-urlencoded; charset=UTF-8""\r\n"
            "Accept: */*""\r\n""\r\n"
            << scontent
            ;
        auto request = sstr.str();

        std::vector<char> data;
        auto login_response_head = httpsSend(socket, request, data, yield);
        std::cout << "\n\tResponse:\n"
            << login_response_head.header_str << std::endl;

        CookieMap cookies;
        saveCookies(login_response_head.headers, cookies);
        // OAuth2 step 1, get code

        xirang2::file_path url("https://developer-dev.api.autodesk.com/authentication/v1/authorize?response_type=code&client_id=5xraq7vtIPiBmY4RdC502oo5FWFL2KFc&redirect_uri=https%3A%2F%2Ficerote.net%2Fblog", xirang2::pp_none);
        xirang2::file_path redirect_site(xirang2::literal("icerote.net"));
        std::string code;
        xirang2::string current_site;
        for (;;) {
            auto uri_info = xirang2::parse_uri(url);
            if (uri_info.site == redirect_site.str()) {
                std::string path = uri_info.path.str().c_str();
                auto pos = xirang2::find(path, '=');
                if (pos != path.end()) ++pos;
                code = std::string(pos, path.end());
                break;
            }
            else if (uri_info.site.empty())
                uri_info.site = current_site;
            else
                current_site = uri_info.site;

            if (uri_info.port.empty())
                uri_info.port = "443";
            
            boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_oauth(io_service, ssl_context);
            connectHttps(uri_info.site.c_str(), socket_oauth, yield, uri_info.port.c_str());

            sstr.str(std::string());
            sstr.clear();

            sstr <<
                "GET " << uri_info.path.str() << " HTTP/1.1""\r\n"
                "Host: "<< uri_info.site << "\r\n"
                "Connection: keep-alive""\r\n"
                << genCookieHeader(cookies) << "\r\n"
                "Accept: */*""\r\n\r\n"
                ;

            request = sstr.str();
            std::cout << "\n\tRequest:\n"
                << request 
                << "\n\twith cookie\n" << genCookieHeader(cookies)
                << std::endl;

            auto res_head = httpsSend(socket_oauth, request, data, yield);
            saveCookies(res_head.headers, cookies);
            std::cout << "\n\tResponse:\n"
                << res_head.header_str << std::endl;

            if (!res_head.code.empty() && res_head.code[0] != '3') 
                break;

            auto itr  = res_head.headers.find("location");
            if (itr != res_head.headers.end()) {
                url = xirang2::file_path(itr->second.c_str(), xirang2::pp_none);
            }
        }

        //// Get Token
        xirang2::file_path token_url("https://developer.api.autodesk.com/authentication/v1/gettoken");
        sstr.str(std::string());
        sstr.clear();

        std::string content = "client_id=5xraq7vtIPiBmY4RdC502oo5FWFL2KFc&"
            "client_secret=gZgY3WzdCrFV8ccC&"
            "grant_type=authorization_code&code=" + code 
            //+ "&redirect_uri=https%3A%2F%2Ficerote.net%2Fblog"
            + "&redirect_uri=https://icerote.net/blog"
            ;

        sstr <<
            "POST /authentication/v1/gettoken HTTP/1.1""\r\n"
            "Host: developer-dev.api.autodesk.com""\r\n"
            "Connection: keep-alive""\r\n"
            "Content-Length : " << content.size() << "\r\n"
            "Content-Type: application/x-www-form-urlencoded""\r\n"
//            << genCookieHeader(cookies) << "\r\n"
            "Accept: */*""\r\n\r\n"
            << content
            ;

        request = sstr.str();
        std::cout << "\n\tRequest:\n"
            << request
            << std::endl;

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket3(io_service, ssl_context);
        connectHttps("developer-dev.api.autodesk.com", socket3, yield);
        auto response = httpsSend(socket3, request, data, yield);
        std::cout << "\n\tResponse:\n"
            << response.header_str << std::endl;


        return;
    });
    try {
        boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23);
        io_service.run();
    }
    catch (xirang2::exception& e) {
        std::cout << "XR Error " << e.what() << std::endl;
    }
    catch (std::exception& e) {
        std::cout << "Error " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "Unknown error\n";
    }
}