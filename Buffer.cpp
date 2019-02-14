#include "Buffer.h"

#include <unistd.h> // write
#include <sys/uio.h> // readv

using namespace swings;

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    // 保证一次读到足够多的数据
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = __begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    const ssize_t n = readv(fd, vec, 2);
    if(n < 0)
        *savedErrno = errno;
    else if(static_cast<size_t>(n) <= writable)
        writerIndex_ += n;
    else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int* savedErrno)
{
    const ssize_t toSend = readableBytes();
    ssize_t nLeft = readableBytes();
    ssize_t nWritten = 0;
    char* bufPtr = __begin() + readerIndex_;

    while(nLeft > 0) {
        if((nWritten = ::write(fd, bufPtr, nLeft) <= 0)) {
            *savedErrno = errno;
            if(errno == EINTR)
                nWritten = 0;
            else
                return -1;
        }
        nLeft -= nWritten;
        bufPtr += nWritten;
    }

    return toSend;
}

