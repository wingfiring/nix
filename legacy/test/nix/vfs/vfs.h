#include <xirang2/vfs.h>

class VfsTester
{
public:
    void test_mount(xirang2::vfs::IVfs& vfs);
    void test_on_empty(xirang2::vfs::IVfs& vfs);
    void test_modification(xirang2::vfs::IVfs& vfs);
    void test_readonly(xirang2::vfs::IVfs& vfs);
};
