#ifndef CONTENTSERVICE_RT_H__
#define CONTENTSERVICE_RT_H__

#include <xirang2/vfs/local.h>
#include <xirang2/vfs/inmemory.h>
#include <xirang2/type/xirang.h>

namespace CS{

struct ContentServiceRTParam
{
    xirang2::string xrName;
    xirang2::string rootfsName;
    xirang2::string workDir;
    xirang2::string resPath;
    xirang2::string dataFile;
    xirang2::string metabasePath;
};

struct ContentServiceRT
{
    xirang2::type::Xirang xr;
    xirang2::vfs::RootFs rootFs;
    xirang2::vfs::LocalFs resourceFs;
    xirang2::vfs::InMemory memFs;
    ContentServiceRTParam params; 
    explicit ContentServiceRT(const ContentServiceRTParam& param);
};

} //namespace

#endif //CONTENTSERVICE_RT_H__
