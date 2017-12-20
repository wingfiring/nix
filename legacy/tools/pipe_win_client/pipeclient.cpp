#ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0501
#endif

#ifdef WIN32
#pragma warning(disable: 4100 4005)
#endif

#include <algorithm>
#include <vector>
#include <sstream>
#include <iomanip>


#include <boost/bind.hpp> 
#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <memory> 


#include <Windows.h>


#include<xirang2/assert.h>

#include <thread>

class session : public std::enable_shared_from_this<session>{
public:
	explicit session(boost::asio::io_service& service, int  session_id)
		: m_io_service(service)
		, m_handle(service)
		, m_id(session_id)
		, m_sequence(0)
	{}

	void read(const std::shared_ptr<std::string>& buffer){
		buffer->resize(138);
		auto asio_buffer = boost::asio::buffer(&(*buffer)[0], buffer->size());
		auto self = shared_from_this();
		boost::asio::async_read(m_handle, asio_buffer, [buffer, self](const boost::system::error_code& err, std::size_t s){
			if (!err){
				buffer->resize(s);
				buffer->push_back(0);
				std::cout << &(*buffer)[0] << "\n";
				buffer->resize(s);
				//self->write();				
			}
		});

	}
	void write(){
		std::stringstream sstr;
		sstr << "Session:" << std::setw(6) << m_id << " Seq:" << std::setw(6) << m_sequence
			<< " >>>update-alternatives: updating alternative /usr/bin/vim.basic because link group vi has changed slave links<<<";

		auto buffer = std::make_shared<std::string>();
		*buffer = sstr.str();

		auto asio_buffer = boost::asio::buffer(&(*buffer)[0], buffer->size());
		auto self = shared_from_this();
		boost::asio::async_write(m_handle, asio_buffer, [buffer, self](boost::system::error_code err, std::size_t){
			if (!err){
				self->read(buffer);
			}
		});

		++m_sequence;
	}

	boost::asio::windows::stream_handle& handle(){
		return m_handle;
	}
private:
	boost::asio::io_service& m_io_service;
	boost::asio::windows::stream_handle m_handle;
	int m_id;
	int m_sequence;
};

void connect(boost::asio::windows::stream_handle& stream, const std::wstring& pipe_name){
	for (;;){
		boost::system::error_code er;
		HANDLE pipe = ::CreateFileW(pipe_name.c_str()
			, GENERIC_READ | GENERIC_WRITE
			, 0		//no sharing
			, NULL
			, OPEN_EXISTING
			, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL
			, NULL
			);
		if (pipe != INVALID_HANDLE_VALUE){
			stream.assign(pipe, er);
			break;
		}
		DWORD const kLastError = ::GetLastError();
		if (kLastError != ERROR_PIPE_BUSY)
			throw boost::system::system_error(kLastError, boost::system::get_system_category());
		BOOL ret = ::WaitNamedPipeW(pipe_name.c_str()
			, NMPWAIT_WAIT_FOREVER);
		if (!ret)
			throw boost::system::system_error(::GetLastError(), boost::system::get_system_category());
	}
}


int main(){
	
	boost::asio::io_service io_service;
	std::unique_ptr<boost::asio::windows::stream_handle_service> 
		stream_handle_service(new boost::asio::windows::stream_handle_service(io_service));
	
	boost::asio::add_service(io_service, stream_handle_service.get());
	stream_handle_service.release();
	
	std::wstring pipe_name(L"\\\\.\\pipe\\test");
	int i = 0;

	std::function<void()> fun;
	fun =  [&](){
		if (++i < 2000000){
			auto s = std::make_shared<session>(io_service, i);
			connect(s->handle(), pipe_name);
			s->write();
			io_service.post(fun);
		}
	};

	io_service.post(fun);

	io_service.run();

	return 0;
}