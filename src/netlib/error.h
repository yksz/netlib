#pragma once

namespace net {

enum struct etype {
    base,
    os,
    netdb,
    ssl,
};

struct error {
    etype type;
    int code;

    bool operator==(const error& other) const {
        return this->type == other.type && this->code == other.code;
    }
    bool operator!=(const error& other) const {
        return this->type != other.type || this->code != other.code;
    }
    bool operator<(const error& other) const {
        return this->code < other.code;
    }

    static const char* Message(const error& err);
    static error wrap(const etype& type, const int& err);


    static const error nil; // success
    static const error unknown;
    static const error eof;
    static const error illegal_argument;
    static const error illegal_state;
    static const error not_found;

    // OS errors (errno)
    static const error perm;
    static const error noent;
    static const error intr;
    static const error io;
    static const error badf;
    static const error nomem;
    static const error acces;
    static const error fault;
    static const error notdir;
    static const error isdir;
    static const error inval;
    static const error nfile;
    static const error mfile;
    static const error notty;
    static const error fbig;
    static const error nospc;
    static const error rofs;
    static const error pipe;
    static const error again;
    static const error wouldblock;
    static const error inprogress;
    static const error already;
    static const error notsock;
    static const error destaddrreq;
    static const error msgsize;
    static const error prototype;
    static const error noprotoopt;
    static const error protonosupport;
    static const error opnotsupp;
    static const error afnosupport;
    static const error addrinuse;
    static const error addrnotavail;
    static const error netdown;
    static const error netunreach;
    static const error netreset;
    static const error connaborted;
    static const error connreset;
    static const error nobufs;
    static const error isconn;
    static const error notconn;
    static const error timedout;
    static const error connrefused;
    static const error loop;
    static const error nametoolong;
    static const error hostunreach;
    static const error proto;

    // OS errors (h_errno)
    static const error host_not_found;
    static const error try_again;
    static const error no_recovery;
    static const error no_data;

    // OpenSSL
    static const error ssl_cert;
};

} // namespace net
