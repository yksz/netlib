#include "net/error.h"

#include <map>
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#else
 #include <cerrno>
 #include <netdb.h>
#endif // defined(_WIN32) || defined(_WIN64)

namespace net {

static const std::map<error, const char*> errorMessages {
    { error::unknown,          "Unknown error" },
    { error::nil,              "No error" },
    { error::eof,              "End of file" },
    { error::illegal_argument, "Illegal argument" },
    { error::illegal_state,    "Illegal state" },
    { error::not_found,        "Not found" },
    { error::perm,             "Operation not permitted" },
    { error::noent,            "No such file or directory" },
    { error::intr,             "Interrupted system call" },
    { error::io,               "Input/output error" },
    { error::badf,             "Bad file descriptor" },
    { error::nomem,            "Cannot allocate memory" },
    { error::acces,            "Permission denied" },
    { error::fault,            "Bad address" },
    { error::notdir,           "Not a directory" },
    { error::isdir,            "Is a directory" },
    { error::inval,            "Invalid argument" },
    { error::nfile,            "Too many open files in system" },
    { error::mfile,            "Too many open files" },
    { error::notty,            "Inappropriate ioctl for device" },
    { error::fbig,             "File too large" },
    { error::nospc,            "No space left on device" },
    { error::rofs,             "Read-only file system" },
    { error::pipe,             "Broken pipe" },
    { error::again,            "Resource temporarily unavailable" },
    { error::wouldblock,       "Operation would block" },
    { error::inprogress,       "Operation now in progress" },
    { error::already,          "Operation already in progress" },
    { error::notsock,          "Socket operation on non-socket" },
    { error::destaddrreq,      "Destination address required" },
    { error::msgsize,          "Message too long" },
    { error::prototype,        "Protocol wrong type for socket" },
    { error::noprotoopt,       "Protocol not available" },
    { error::protonosupport,   "Protocol not supported" },
    { error::opnotsupp,        "Operation not supported" },
    { error::afnosupport,      "Address family not supported by protocol family" },
    { error::addrinuse,        "Address already in use" },
    { error::addrnotavail,     "Can't assign requested address" },
    { error::netdown,          "Network is down" },
    { error::netunreach,       "Network is unreachable" },
    { error::netreset,         "Network dropped connection on reset" },
    { error::connaborted,      "Software caused connection abort" },
    { error::connreset,        "Connection reset by peer" },
    { error::nobufs,           "No buffer space available" },
    { error::isconn,           "Socket is already connected" },
    { error::notconn,          "Socket is not connected" },
    { error::timedout,         "Operation timed out" },
    { error::connrefused,      "Connection refused" },
    { error::loop,             "Too many levels of symbolic links" },
    { error::nametoolong,      "File name too long" },
    { error::hostunreach,      "No route to host" },
    { error::proto,            "Protocol error" },
    { error::host_not_found,   "No such host is known" },
    { error::try_again,        "A temporary error occurred on an authoritative name server" },
    { error::no_recovery,      "A nonrecoverable name server error occurred" },
    { error::no_data,          "The requested name is valid but does not have an IP address" },
};

#if defined(_WIN32) || defined(_WIN64)
static const std::map<int, error> osErrors {
    { WSAEINTR,           error::intr },
    { WSAEBADF,           error::badf },
    { WSAEACCES,          error::acces },
    { WSAEFAULT,          error::fault },
    { WSAEINVAL,          error::inval },
    { WSAEMFILE,          error::mfile },
    { WSAEWOULDBLOCK,     error::wouldblock },
    { WSAEINPROGRESS,     error::inprogress },
    { WSAEALREADY,        error::already },
    { WSAENOTSOCK,        error::notsock },
    { WSAEDESTADDRREQ,    error::destaddrreq },
    { WSAEMSGSIZE,        error::msgsize },
    { WSAEPROTOTYPE,      error::prototype },
    { WSAENOPROTOOPT,     error::noprotoopt },
    { WSAEPROTONOSUPPORT, error::protonosupport },
    { WSAEOPNOTSUPP,      error::opnotsupp },
    { WSAEAFNOSUPPORT,    error::afnosupport },
    { WSAEADDRINUSE,      error::addrinuse },
    { WSAEADDRNOTAVAIL,   error::addrnotavail },
    { WSAENETDOWN,        error::netdown },
    { WSAENETUNREACH,     error::netunreach },
    { WSAENETRESET,       error::netreset },
    { WSAECONNABORTED,    error::connaborted },
    { WSAECONNRESET,      error::connreset },
    { WSAENOBUFS,         error::nobufs },
    { WSAEISCONN,         error::isconn },
    { WSAENOTCONN,        error::notconn },
    { WSAETIMEDOUT,       error::timedout },
    { WSAECONNREFUSED,    error::connrefused },
    { WSAELOOP,           error::loop },
    { WSAENAMETOOLONG,    error::nametoolong },
    { WSAEHOSTUNREACH,    error::hostunreach },
    { WSAHOST_NOT_FOUND,  error::host_not_found },
    { WSATRY_AGAIN,       error::try_again },
    { WSANO_RECOVERY,     error::no_recovery },
};
#else
static const std::map<int, error> osErrors {
    { EPERM,           error::perm },
    { ENOENT,          error::noent },
    { EINTR,           error::intr },
    { EIO,             error::io },
    { EBADF,           error::badf },
    { ENOMEM,          error::nomem },
    { EACCES,          error::acces },
    { EFAULT,          error::fault },
    { ENOTDIR,         error::notdir },
    { EISDIR,          error::isdir },
    { EINVAL,          error::inval },
    { ENFILE,          error::nfile },
    { EMFILE,          error::mfile },
    { ENOTTY,          error::notty },
    { EFBIG,           error::fbig },
    { ENOSPC,          error::nospc },
    { EROFS,           error::rofs },
    { EPIPE,           error::pipe },
    { EAGAIN,          error::again },
    { EWOULDBLOCK,     error::wouldblock },
    { EINPROGRESS,     error::inprogress },
    { EALREADY,        error::already },
    { ENOTSOCK,        error::notsock },
    { EDESTADDRREQ,    error::destaddrreq },
    { EMSGSIZE,        error::msgsize },
    { EPROTOTYPE,      error::prototype },
    { ENOPROTOOPT,     error::noprotoopt },
    { EPROTONOSUPPORT, error::protonosupport },
    { EOPNOTSUPP,      error::opnotsupp },
    { EAFNOSUPPORT,    error::afnosupport },
    { EADDRINUSE,      error::addrinuse },
    { EADDRNOTAVAIL,   error::addrnotavail },
    { ENETDOWN,        error::netdown },
    { ENETUNREACH,     error::netunreach },
    { ENETRESET,       error::netreset },
    { ECONNABORTED,    error::connaborted },
    { ECONNRESET,      error::connreset },
    { ENOBUFS,         error::nobufs },
    { EISCONN,         error::isconn },
    { ENOTCONN,        error::notconn },
    { ETIMEDOUT,       error::timedout },
    { ECONNREFUSED,    error::connrefused },
    { ELOOP,           error::loop },
    { ENAMETOOLONG,    error::nametoolong },
    { EHOSTUNREACH,    error::hostunreach },
    { EPROTO,          error::proto },
    { EAI_NONAME,      error::host_not_found },
    { EAI_AGAIN,       error::try_again },
    { EAI_FAIL,        error::no_recovery },
    { EAI_NODATA,      error::no_data },
};
#endif // defined(_WIN32) || defined(_WIN64)

const char* GetErrorMessage(const error& err) {
    auto it = errorMessages.find(err);
    return it != errorMessages.end() ? it->second : "";
}

error GetOSError(const int& osErrno) {
    if (osErrno == 0) {
        return error::nil;
    }
    auto it = osErrors.find(osErrno);
    return it != osErrors.end() ? it->second : error::unknown;
}

} // namespace net
