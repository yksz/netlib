#pragma once

namespace cx {

enum struct error {
    unknown = 0,
    nil, // success
    eof,
    illegal_argument,
    illegal_state,
    not_found,

    // OS errors (errno)
    perm,
    noent,
    intr,
    io,
    badf,
    nomem,
    acces,
    fault,
    notdir,
    isdir,
    inval,
    nfile,
    mfile,
    notty,
    fbig,
    nospc,
    rofs,
    pipe,
    again,
    wouldblock,
    inprogress,
    already,
    notsock,
    destaddrreq,
    msgsize,
    prototype,
    noprotoopt,
    protonosupport,
    opnotsupp,
    afnosupport,
    addrinuse,
    addrnotavail,
    netdown,
    netunreach,
    netreset,
    connaborted,
    connreset,
    nobufs,
    isconn,
    notconn,
    timedout,
    connrefused,
    loop,
    nametoolong,
    hostunreach,
    proto,

    // OS errors (h_errno)
    host_not_found,
    try_again,
    no_recovery,
    no_data
};

const char* GetErrorMessage(const error& err);
error GetOSError(const int& osErrno);

} // namespace cx
