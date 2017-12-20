#ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0501
#endif

#ifdef WIN32
#pragma warning(disable: 4100 4005)
#endif

#include <algorithm>
#include <vector>


#include <boost/bind.hpp> 
#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <memory> 


#include <Windows.h>


#include<xirang2/assert.h>


//----------------------------------------------------------------------------------------------
// class pipe_acceptor
//
class pipe_acceptor
{
	boost::asio::io_service&    m_service;
	std::wstring m_pipename;
	DWORD m_bufferSize;
	public: // Interface
	pipe_acceptor(boost::asio::io_service& ioService, const std::wstring& pipename, std::size_t buffer_size)
		: m_service(ioService)
		, m_pipename(pipename)
		, m_bufferSize(DWORD(buffer_size))
	{	
	}

	~pipe_acceptor(void)
	{
	}


	template<typename Handler>
	void async_accept(boost::asio::windows::stream_handle& stream, Handler handler)
	{
		HANDLE handle = ::CreateNamedPipeW
			(
			m_pipename.c_str()
			, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED
			, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT
			, PIPE_UNLIMITED_INSTANCES
			, DWORD(m_bufferSize)
			, DWORD(m_bufferSize)
			, NMPWAIT_USE_DEFAULT_WAIT
			, NULL
			);
		if (handle == INVALID_HANDLE_VALUE)
		{
			throw boost::system::system_error(::GetLastError(), boost::system::get_system_category());
		}

		stream.assign(handle/*, er*/);
		boost::asio::windows::overlapped_ptr overlapped(m_service, 
			[&stream, handle, handler](boost::system::error_code const& err, size_t ){
			handler(err);
		});

		BOOL result = ::ConnectNamedPipe(handle, overlapped.get());
		DWORD const kLastError = GetLastError();
		if (!result && (kLastError != ERROR_IO_PENDING) && (kLastError != ERROR_PIPE_CONNECTED))
		{
			boost::system::error_code error(kLastError, boost::asio::error::get_system_category());
			overlapped.complete(error, 0);
			::DisconnectNamedPipe(handle);
		}
		else
		{
			overlapped.release();
		}
	}

}; // class pipe_acceptor

class session : public std::enable_shared_from_this<session>{
public:
	explicit session(boost::asio::io_service& service)
		: m_io_service(service)
		, m_handle(service)
	{}

	void read(){
		auto buffer = std::make_shared<std::vector<char>>();
		buffer->resize(138);
		auto asio_buffer = boost::asio::buffer(&(*buffer)[0], buffer->size());
		auto self = shared_from_this();
		boost::asio::async_read(m_handle, asio_buffer, [buffer, self](const boost::system::error_code& err, std::size_t s){
			if (!err){
				buffer->resize(s);
				buffer->push_back(0);
				std::cout << &(*buffer)[0] << "\n";
				buffer->resize(s);
				self->write(buffer);
			}
		});
		
	}
	void write(const std::shared_ptr<std::vector<char>>& buffer){
		auto asio_buffer = boost::asio::buffer(&(*buffer)[0], buffer->size());
		auto self = shared_from_this();
		boost::asio::async_write(m_handle, asio_buffer, [self](boost::system::error_code err, std::size_t){
			if (!err){
				self->read();
			}
		});
	}
	
	boost::asio::windows::stream_handle& handle(){
		return m_handle;
	}
private:
	boost::asio::io_service& m_io_service;
	boost::asio::windows::stream_handle m_handle;
};

class server{
	boost::asio::io_service& io_service;
	pipe_acceptor acceptor;
public:
	server(boost::asio::io_service& ios) 
		: io_service(ios)
		, acceptor(io_service, L"\\\\.\\pipe\\test", 32768)
	{	
	}

	void start(){
		int sequence = ++m_sequence;

		std::cout << "Listen " << sequence << "\n";
		io_service.post([this, sequence]{
			auto s = std::make_shared<session>(io_service);
			acceptor.async_accept(s->handle(), [this, s, sequence](boost::system::error_code const& /*err*/){
				std::cout << "accepted " << sequence << "\n";
				this->start();
				s->read();
			});
		});
	}
	int m_sequence = 0;
};

int main(){
	boost::asio::io_service io_service;
	std::unique_ptr<boost::asio::windows::stream_handle_service> 
		stream_handle_service(new boost::asio::windows::stream_handle_service(io_service));
	boost::asio::add_service(io_service, stream_handle_service.get());
	stream_handle_service.release();
	server s(io_service);
	s.start();	

	io_service.run();
	return 0;
}