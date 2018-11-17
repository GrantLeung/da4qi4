#ifndef DAQI_APPLICATION_HPP
#define DAQI_APPLICATION_HPP

#include <set>
#include <map>
#include <memory>

#include "def/boost_def.hpp"
#include "utilities/string_utilities.hpp"

#include "router.hpp"
#include "templates.hpp"
#include "intercepter.hpp"

namespace da4qi4
{

struct UploadFileSaveOptions
{
    enum Strategy
    {
        always_save
        , alway_no_save
        , size_greater_than
        , size_lesser_than
        , extension_is
        , extension_is_not
    };

    bool IsNeedSave(std::string const& extension, size_t filesize_kb) const;

    Strategy strategy = always_save;
    size_t size_base_kb;
    std::set<std::string, Utilities::IgnoreCaseCompare> extensions;
};

class Application
{
private:
    Application();

    Application(std::string const& name
                , std::string const& root_url
                , fs::path const& root_static
                , fs::path const& root_template
                , fs::path const& root_upload
                , fs::path const& root_temporary
               );
public:
    static std::shared_ptr<Application> Default()
    {
        return std::shared_ptr<Application>(new Application());
    }

    static std::shared_ptr<Application> Customize(std::string const& name
                                                  , std::string const& root_url
                                                  , fs::path const& root_static = ""
                                                  , fs::path const& root_template = ""
                                                  , fs::path const& root_upload = ""
                                                  , fs::path const& root_temporary = "")
    {
        return std::shared_ptr<Application>(new Application(name,
                                                            root_url, root_static,
                                                            root_template, root_upload,
                                                            root_temporary));
    }

    ~Application();

    Application& SetStaticRoot(std::string const& root_static)
    {
        _root_static = root_static;
        return *this;
    }

    void Mount()
    {
        _mounted = true;
    }

    bool IsRuning() const
    {
        return  _mounted && !_disabled;
    }

    Application& SetTemplateRoot(std::string const& root_template)
    {
        if (!IsRuning())
        {
            _root_template = root_template;
        }

        return *this;
    }

    Application& SetDefaultCharset(std::string const& charset)
    {
        if (!IsRuning())
        {
            _default_charset = charset;
        }

        return *this;
    }

    Application& SetUploadRoot(std::string const& root_upload)
    {
        if (!IsRuning())
        {
            _root_upload = root_upload;
        }

        return *this;
    }

    std::string const& GetName() const
    {
        return _name;
    }

    std::string const& GetDefaultCharset() const
    {
        return _default_charset;
    }

    std::string const& GetUrlRoot() const
    {
        return _root_url;
    }

    fs::path const& GetStaticRootPath() const
    {
        return _root_static;
    }

    fs::path const& GetTemplateRoot() const
    {
        return _root_template;
    }

    fs::path const& GetUploadRoot() const
    {
        return _root_upload;
    }

    bool IsEnable() const
    {
        return !_disabled;
    }
    void Disable();
    void Enable();

    void CheckUpdate();

    size_t GetUpoadMaxSizeLimitKB() const
    {
        return _upload_max_size_limit_kb;
    }

    void SetUpoadMaxSizeLimitKB(size_t size_limit_kb)
    {
        _upload_max_size_limit_kb = size_limit_kb;
    }

    UploadFileSaveOptions const& GetUploadFileSaveOptions() const
    {
        return _upload_file_save_opt;
    }

    UploadFileSaveOptions& GetUploadFileSaveOptions()
    {
        return _upload_file_save_opt;
    }

    Templates const& GetTemplates() const
    {
        return _templates;
    }

    Templates& GetTemplates()
    {
        return _templates;
    }

    void Handle(Context ctx);

public:
    bool AddHandler(HandlerMethod m, std::string const& url, Handler h)
    {
        if (IsRuning())
        {
            return false;
        }

        return AddHandler(m, router_equals(url), h);
    }

    bool AddHandler(HandlerMethod m, router_equals r, Handler h);
    bool AddHandler(HandlerMethod m, router_starts r, Handler h);
    bool AddHandler(HandlerMethod m, router_regex r, Handler h);

    bool AddHandler(HandlerMethods ms, std::string const& url, Handler h)
    {
        if (IsRuning())
        {
            return false;
        }

        return AddHandler(ms, router_equals(url), h);
    }
    bool AddHandler(HandlerMethods ms, router_equals e, Handler h);
    bool AddHandler(HandlerMethods ms, router_starts e, Handler h);
    bool AddHandler(HandlerMethods ms, router_regex e, Handler h);

public:
    void AddIntercepter(Intercepter::Handler intercepter)
    {
        if (!IsRuning())
        {
            _intercepters.push_back(intercepter);
        }
    }

    std::pair<Intercepter::ChainIterator, Intercepter::ChainIterator>
    GetIntercepterChainRange()
    {
        return { _intercepters.begin(), _intercepters.end() };
    }

private:
    void init_pathes();

private:
    Handler& find_handler(const Context& ctx);
    void do_handle(Context& ctx);
private:
    EqualsRoutingTable _equalRouter;
    StartsWithRoutingTable _startwithsRouter;
    RegexMatchRoutingTable _regexRouter;

private:
    bool _disabled = false;
    bool _mounted = false;

    std::string _name;
    std::string _default_charset = "UTF-8";

    std::string _root_url;
    fs::path _root_static;
    fs::path _root_template;
    fs::path _root_upload;
    fs::path _root_temporary;

    size_t _upload_max_size_limit_kb = 1 * 1024 * 1024;

    Templates _templates;

    UploadFileSaveOptions _upload_file_save_opt;

private:
    Intercepter::Chain _intercepters;
};

using ApplicationPtr = std::shared_ptr<Application>;

using ApplicationMap = std::map<std::string /* url */
                       , ApplicationPtr, Utilities::IgnoreCaseCompareDESC>;

class ApplicationMgr
{
public:
    bool CreateDefaultIfEmpty();
    bool Add(ApplicationPtr app);

    void Enable(std::string const& name);
    void Disable(std::string const& name);

    bool IsExists(std::string const& name) const;
    bool IsEnable(std::string const& name) const;

    ApplicationPtr FindByURL(std::string const& url);

    ApplicationPtr FindByName(std::string const& name);
    ApplicationPtr const FindByName(std::string const& name) const;

    ApplicationMap const& All() const
    {
        return _map;
    }

    void Mount();
    void CheckTemplatesUpdate();

    bool IsEmpty() const
    {
        return _map.empty();
    }
    size_t Count() const
    {
        return _map.size();
    }

private:
    ApplicationMap _map;
    bool _mounted = false;
};

ApplicationMgr& AppMgr();

} // namespace da4qi4

#endif // DAQI_APPLICATION_HPP