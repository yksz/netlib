#include "net/error.h"

#include <map>
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#else
 #include <cerrno>
 #include <netdb.h>
#endif // defined(_WIN32) || defined(_WIN64)
#include <openssl/err.h>

namespace net {

const error error::unknown          = { etype::base, 0 };
const error error::nil              = { etype::base, 1 };
const error error::eof              = { etype::base, 2 };
const error error::illegal_argument = { etype::base, 3 };
const error error::illegal_state    = { etype::base, 4 };
const error error::not_found        = { etype::base, 5 };

#if defined(_WIN32) || defined(_WIN64)
const error error::perm           = { etype::os, ERROR_ACCESS_DENIED };
const error error::noent          = { etype::os, ENOENT              };
const error error::intr           = { etype::os, WSAEINTR            };
const error error::io             = { etype::os, EIO                 };
const error error::badf           = { etype::os, WSAEBADF            };
const error error::nomem          = { etype::os, ERROR_OUTOFMEMORY   };
const error error::acces          = { etype::os, WSAEACCES           };
const error error::fault          = { etype::os, WSAEFAULT           };
const error error::notdir         = { etype::os, ENOTDIR             };
const error error::isdir          = { etype::os, EISDIR              };
const error error::inval          = { etype::os, WSAEINVAL           };
const error error::nfile          = { etype::os, ENFILE              };
const error error::mfile          = { etype::os, WSAEMFILE           };
const error error::notty          = { etype::os, ENOTTY              };
const error error::fbig           = { etype::os, EFBIG               };
const error error::nospc          = { etype::os, ENOSPC              };
const error error::rofs           = { etype::os, EROFS               };
const error error::pipe           = { etype::os, ERROR_BROKEN_PIPE   };
const error error::again          = { etype::os, ERROR_RETRY         };
const error error::wouldblock     = { etype::os, WSAEWOULDBLOCK      };
const error error::inprogress     = { etype::os, WSAEINPROGRESS      };
const error error::already        = { etype::os, WSAEALREADY         };
const error error::notsock        = { etype::os, WSAENOTSOCK         };
const error error::destaddrreq    = { etype::os, WSAEDESTADDRREQ     };
const error error::msgsize        = { etype::os, WSAEMSGSIZE         };
const error error::prototype      = { etype::os, WSAEPROTOTYPE       };
const error error::noprotoopt     = { etype::os, WSAENOPROTOOPT      };
const error error::protonosupport = { etype::os, WSAEPROTONOSUPPORT  };
const error error::opnotsupp      = { etype::os, WSAEOPNOTSUPP       };
const error error::afnosupport    = { etype::os, WSAEAFNOSUPPORT     };
const error error::addrinuse      = { etype::os, WSAEADDRINUSE       };
const error error::addrnotavail   = { etype::os, WSAEADDRNOTAVAIL    };
const error error::netdown        = { etype::os, WSAENETDOWN         };
const error error::netunreach     = { etype::os, WSAENETUNREACH      };
const error error::netreset       = { etype::os, WSAENETRESET        };
const error error::connaborted    = { etype::os, WSAECONNABORTED     };
const error error::connreset      = { etype::os, WSAECONNRESET       };
const error error::nobufs         = { etype::os, WSAENOBUFS          };
const error error::isconn         = { etype::os, WSAEISCONN          };
const error error::notconn        = { etype::os, WSAENOTCONN         };
const error error::timedout       = { etype::os, WSAETIMEDOUT        };
const error error::connrefused    = { etype::os, WSAECONNREFUSED     };
const error error::loop           = { etype::os, WSAELOOP            };
const error error::nametoolong    = { etype::os, WSAENAMETOOLONG     };
const error error::hostunreach    = { etype::os, WSAEHOSTUNREACH     };
const error error::proto          = { etype::os, EPROTO              };
const error error::host_not_found = { etype::netdb, WSAHOST_NOT_FOUND };
const error error::no_data        = { etype::netdb, WSANO_DATA        };
const error error::no_recovery    = { etype::netdb, WSANO_RECOVERY    };
const error error::try_again      = { etype::netdb, WSATRY_AGAIN      };
#else
const error error::perm           = { etype::os, EPERM           };
const error error::noent          = { etype::os, ENOENT          };
const error error::intr           = { etype::os, EINTR           };
const error error::io             = { etype::os, EIO             };
const error error::badf           = { etype::os, EBADF           };
const error error::nomem          = { etype::os, ENOMEM          };
const error error::acces          = { etype::os, EACCES          };
const error error::fault          = { etype::os, EFAULT          };
const error error::notdir         = { etype::os, ENOTDIR         };
const error error::isdir          = { etype::os, EISDIR          };
const error error::inval          = { etype::os, EINVAL          };
const error error::nfile          = { etype::os, ENFILE          };
const error error::mfile          = { etype::os, EMFILE          };
const error error::notty          = { etype::os, ENOTTY          };
const error error::fbig           = { etype::os, EFBIG           };
const error error::nospc          = { etype::os, ENOSPC          };
const error error::rofs           = { etype::os, EROFS           };
const error error::pipe           = { etype::os, EPIPE           };
const error error::again          = { etype::os, EAGAIN          };
const error error::wouldblock     = { etype::os, EWOULDBLOCK     };
const error error::inprogress     = { etype::os, EINPROGRESS     };
const error error::already        = { etype::os, EALREADY        };
const error error::notsock        = { etype::os, ENOTSOCK        };
const error error::destaddrreq    = { etype::os, EDESTADDRREQ    };
const error error::msgsize        = { etype::os, EMSGSIZE        };
const error error::prototype      = { etype::os, EPROTOTYPE      };
const error error::noprotoopt     = { etype::os, ENOPROTOOPT     };
const error error::protonosupport = { etype::os, EPROTONOSUPPORT };
const error error::opnotsupp      = { etype::os, EOPNOTSUPP      };
const error error::afnosupport    = { etype::os, EAFNOSUPPORT    };
const error error::addrinuse      = { etype::os, EADDRINUSE      };
const error error::addrnotavail   = { etype::os, EADDRNOTAVAIL   };
const error error::netdown        = { etype::os, ENETDOWN        };
const error error::netunreach     = { etype::os, ENETUNREACH     };
const error error::netreset       = { etype::os, ENETRESET       };
const error error::connaborted    = { etype::os, ECONNABORTED    };
const error error::connreset      = { etype::os, ECONNRESET      };
const error error::nobufs         = { etype::os, ENOBUFS         };
const error error::isconn         = { etype::os, EISCONN         };
const error error::notconn        = { etype::os, ENOTCONN        };
const error error::timedout       = { etype::os, ETIMEDOUT       };
const error error::connrefused    = { etype::os, ECONNREFUSED    };
const error error::loop           = { etype::os, ELOOP           };
const error error::nametoolong    = { etype::os, ENAMETOOLONG    };
const error error::hostunreach    = { etype::os, EHOSTUNREACH    };
const error error::proto          = { etype::os, EPROTO          };
const error error::host_not_found = { etype::netdb, EAI_NONAME };
const error error::no_data        = { etype::netdb, EAI_NODATA };
const error error::no_recovery    = { etype::netdb, EAI_FAIL   };
const error error::try_again      = { etype::netdb, EAI_AGAIN  };
#endif // defined(_WIN32) || defined(_WIN64)
const error error::ssl_cert       = { etype::ssl, -1 };

static const std::map<const error, const char*> emessages {
    { error::unknown,          "Unknown error"     },
    { error::nil,              "No error"          },
    { error::eof,              "End of file"       },
    { error::illegal_argument, "Illegal argument"  },
    { error::illegal_state,    "Illegal state"     },
    { error::not_found,        "Element not found" },
    { error::perm,             "Operation not permitted"                         },
    { error::noent,            "No such file or directory"                       },
    { error::intr,             "Interrupted system call"                         },
    { error::io,               "Input/output error"                              },
    { error::badf,             "Bad file descriptor"                             },
    { error::nomem,            "Cannot allocate memory"                          },
    { error::acces,            "Permission denied"                               },
    { error::fault,            "Bad address"                                     },
    { error::notdir,           "Not a directory"                                 },
    { error::isdir,            "Is a directory"                                  },
    { error::inval,            "Invalid argument"                                },
    { error::nfile,            "Too many open files in system"                   },
    { error::mfile,            "Too many open files"                             },
    { error::notty,            "Inappropriate ioctl for device"                  },
    { error::fbig,             "File too large"                                  },
    { error::nospc,            "No space left on device"                         },
    { error::rofs,             "Read-only file system"                           },
    { error::pipe,             "Broken pipe"                                     },
    { error::again,            "Resource temporarily unavailable"                },
    { error::wouldblock,       "Operation would block"                           },
    { error::inprogress,       "Operation now in progress"                       },
    { error::already,          "Operation already in progress"                   },
    { error::notsock,          "Socket operation on non-socket"                  },
    { error::destaddrreq,      "Destination address required"                    },
    { error::msgsize,          "Message too long"                                },
    { error::prototype,        "Protocol wrong type for socket"                  },
    { error::noprotoopt,       "Protocol not available"                          },
    { error::protonosupport,   "Protocol not supported"                          },
    { error::opnotsupp,        "Operation not supported"                         },
    { error::afnosupport,      "Address family not supported by protocol family" },
    { error::addrinuse,        "Address already in use"                          },
    { error::addrnotavail,     "Can't assign requested address"                  },
    { error::netdown,          "Network is down"                                 },
    { error::netunreach,       "Network is unreachable"                          },
    { error::netreset,         "Network dropped connection on reset"             },
    { error::connaborted,      "Software caused connection abort"                },
    { error::connreset,        "Connection reset by peer"                        },
    { error::nobufs,           "No buffer space available"                       },
    { error::isconn,           "Socket is already connected"                     },
    { error::notconn,          "Socket is not connected"                         },
    { error::timedout,         "Operation timed out"                             },
    { error::connrefused,      "Connection refused"                              },
    { error::loop,             "Too many levels of symbolic links"               },
    { error::nametoolong,      "File name too long"                              },
    { error::hostunreach,      "No route to host"                                },
    { error::proto,            "Protocol error"                                  },
    { error::host_not_found,   "No such host is known"                                       },
    { error::no_data,          "The requested name is valid but does not have an IP address" },
    { error::no_recovery,      "A nonrecoverable name server error occurred"                 },
    { error::try_again,        "A temporary error occurred on an authoritative name server"  },
    { error::ssl_cert,         "Certificate error" },
};

const char* error::Message(const error& err) {
    if (err.type == etype::ssl) {
        const char* s = ERR_reason_error_string(err.code);
        if (s != nullptr) {
            return s;
        }
    }
    auto it = emessages.find(err);
    if (it != emessages.end()) {
        return it->second;
    }
    switch (err.type) {
        case etype::base:
            return "base error";
        case etype::os:
            return "os error";
        case etype::netdb:
            return "netdb error";
        case etype::ssl:
            return "ssl error";
        default:
            return "";
    }
}

error error::wrap(const etype& type, const int& err) {
    if (err == 0) {
        return error::nil;
    }
    return { type, err };
}

} // namespace net
