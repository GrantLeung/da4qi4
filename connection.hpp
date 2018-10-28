#ifndef Connection_HPP
#define Connection_HPP

#include <memory>

#include "http-parser/http_parser.h"
#include "multipart-parser/multipart_parser.h"

#include "def/asio_def.hpp"

#include "request.hpp"
#include "response.hpp"
#include "handler.hpp"
#include "application.hpp"

namespace da4qi4
{

class Connection
    : public std::enable_shared_from_this<Connection>
{
public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    
    explicit Connection(Tcp::socket socket);
    ~Connection();
    
    void Start();
    void Stop();
    void Write();
public:
    Request const& GetRequest() { return _request; }
    Response& GetResponse() { return _response; }
    Application& GetApplication() { return (_app ? *_app : Application::EmptyApplication()); }
    
private:
    void do_read();
    void do_write();
    void do_close();
    
private:
    Tcp::socket _socket;
    std::array<char, 1024 * 4> _buffer;
    
private:
    void init_parser();
    void init_parser_setting();
    
    static int on_header_field(http_parser* parser, char const* at, size_t length);
    static int on_header_value(http_parser* parser, char const* at, size_t length);
    static int on_headers_complete(http_parser* parser);
    
    static int on_message_begin(http_parser* parser);
    static int on_message_complete(http_parser* parser);
    
    static int on_url(http_parser* parser, char const* at, size_t length);
    static int on_body(http_parser* parser, char const* at, size_t length);
    
private:
    static int on_multipart_header_field(multipart_parser* parser, char const* at, size_t length);
    static int on_multipart_header_value(multipart_parser* parser, char const* at, size_t length);
    static int on_multipart_headers_complete(multipart_parser* parser);
    
    static int on_multipart_data_begin(multipart_parser* parser);
    static int on_multipart_data(multipart_parser* parser, char const* at, size_t length);
    static int on_multipart_data_end(multipart_parser* parser);
    static int on_multipart_body_end(multipart_parser* parser);
    
private:
    void update_request_after_header_parsed();
    
    void try_commit_reading_request_header();
    void process_100_continue_request();
    
    void try_fix_multipart_bad_request_without_boundary();
    void try_init_multipart_parser();
    
    enum MultpartParseStatus { mp_cannot_init = -1, mp_parsing = 0,  mp_parse_fail = 1};
    MultpartParseStatus do_multipart_parse();
    
    bool try_route_application();
private:
    void prepare_response_headers_about_connection();
    
private:
    http_parser* _parser;
    http_parser_settings _parser_setting;
    
    std::string  _url;
    
    enum ReadingHeaderPart {header_none_part, header_field_part, header_value_part};
    ReadingHeaderPart  _reading_header_part = header_none_part;
    std::string _reading_header_field, _reading_header_value;
    
    std::string _body;
    
    enum ReadCompletePart {read_none_complete, read_header_complete, read_message_complete};
    ReadCompletePart _read_complete = read_none_complete;
    
private:
    void init_multipart_parser(std::string const& boundary);
    enum mp_free_flag  {will_free_mp_setting = 1, will_free_mp_parser = 2, will_free_mp_both = 3 };
    void free_multipart_parser(mp_free_flag flag = will_free_mp_both);
    
    multipart_parser_settings* _mp_parser_setting = nullptr;
    multipart_parser* _mp_parser = nullptr;
    MultiPart _reading_part;
    std::string _reading_part_data;
    
    boost::asio::streambuf _write_buffer;
private:
    Request _request;
    Response _response;
    Application* _app = nullptr;
};

} //namespace da4qi4

#endif // Connection_HPP
