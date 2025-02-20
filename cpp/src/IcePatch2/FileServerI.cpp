//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>
#include <IcePatch2/FileServerI.h>

#ifdef _WIN32
#    include <io.h>
#else
#    include <unistd.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;
using namespace IcePatch2Internal;

IcePatch2::FileServerI::FileServerI(const std::string& dataDir, const LargeFileInfoSeq& infoSeq)
    : _dataDir(dataDir),
      _tree0(FileTree0())
{
    FileTree0& tree0 = const_cast<FileTree0&>(_tree0);
    getFileTree0(infoSeq, tree0);
}

LargeFileInfoSeq
IcePatch2::FileServerI::getLargeFileInfoSeq(int32_t node0, const Current&) const
{
    if (node0 < 0 || node0 > 255)
    {
        throw PartitionOutOfRangeException();
    }

    return _tree0.nodes[static_cast<size_t>(node0)].files;
}

ByteSeqSeq
IcePatch2::FileServerI::getChecksumSeq(const Current&) const
{
    ByteSeqSeq checksums(256);

    for (size_t node0 = 0; node0 < 256; ++node0)
    {
        checksums[node0] = _tree0.nodes[node0].checksum;
    }

    return checksums;
}

ByteSeq
IcePatch2::FileServerI::getChecksum(const Current&) const
{
    return _tree0.checksum;
}

void
IcePatch2::FileServerI::getLargeFileCompressedAsync(
    string pa,
    int64_t pos,
    int32_t num,
    function<void(pair<const byte*, const byte*> returnValue)> response,
    function<void(exception_ptr)> exception,
    const Current&) const
{
    try
    {
        vector<byte> buffer;
        getFileCompressedInternal(std::move(pa), pos, num, buffer, true);
        if (buffer.empty())
        {
            response(make_pair<const byte*, const byte*>(0, 0));
        }
        else
        {
            response(make_pair<const byte*, const byte*>(&buffer[0], &buffer[0] + buffer.size()));
        }
    }
    catch (const std::exception&)
    {
        exception(current_exception());
    }
}

void
IcePatch2::FileServerI::getFileCompressedInternal(
    std::string pa,
    int64_t pos,
    int32_t num,
    vector<byte>& buffer,
    bool largeFile) const
{
    if (IceUtilInternal::isAbsolutePath(pa))
    {
        throw FileAccessException(string("illegal absolute path `") + pa + "'");
    }

    string path = simplify(pa);

    if (path == ".." || path.find("/../") != string::npos ||
        (path.size() >= 3 && (path.substr(0, 3) == "../" || path.substr(path.size() - 3, 3) == "/..")))
    {
        throw FileAccessException(string("illegal `..' component in path `") + path + "'");
    }

    if (num <= 0 || pos < 0)
    {
        return;
    }

    string absolutePath = _dataDir + '/' + path + ".bz2";
    int fd = IceUtilInternal::open(absolutePath, O_RDONLY | O_BINARY);
    if (fd == -1)
    {
        throw FileAccessException(
            string("cannot open `") + path + "' for reading: " + IceUtilInternal::errorToString(errno));
    }

    if (!largeFile)
    {
        IceUtilInternal::structstat buf;
        if (IceUtilInternal::stat(absolutePath, &buf) == -1)
        {
            throw FileAccessException(string("cannot stat `") + path + "':\n" + IceUtilInternal::lastErrorToString());
        }

        if (buf.st_size > 0x7FFFFFFF)
        {
            ostringstream os;
            os << "cannot encode size `" << buf.st_size << "' for file `" << path << "' as int32_t" << endl;
            throw FileAccessException(os.str());
        }
    }

    if (
#if defined(_MSC_VER)
        _lseek(fd, static_cast<off_t>(pos), SEEK_SET)
#else
        lseek(fd, static_cast<off_t>(pos), SEEK_SET)
#endif
        != static_cast<off_t>(pos))
    {
        IceUtilInternal::close(fd);

        ostringstream posStr;
        posStr << pos;

        throw FileAccessException(
            "cannot seek position " + posStr.str() + " in file `" + path +
            "': " + IceUtilInternal::errorToString(errno));
    }

    buffer.resize(static_cast<size_t>(num));
#ifdef _WIN32
    int r;
    if ((r = _read(fd, &buffer[0], static_cast<unsigned int>(num))) == -1)
#else
    ssize_t r;
    if ((r = read(fd, &buffer[0], static_cast<size_t>(num))) == -1)
#endif
    {
        IceUtilInternal::close(fd);
        throw FileAccessException("cannot read `" + path + "': " + IceUtilInternal::errorToString(errno));
    }

    IceUtilInternal::close(fd);
}
