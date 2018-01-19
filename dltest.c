#define _GNU_SOURCE
#include <dlfcn.h>
#include <sysexits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

void printinfo(const char* restrict name, void* handle)
{
#define assert_not(stmt, what) \
    do { if ((what) == (stmt)) { return; } } while (0) 
#define assert_not_do(stmt, what, dothing) \
    do { if ((what) == (stmt)) { dothing; return; } } while (0)

    Dl_serinfo serinfo = {0};
    Dl_serinfo *sip = NULL;

    assert_not(dlinfo(handle, RTLD_DI_SERINFOSIZE, &serinfo), -1);
    assert_not(sip = calloc(serinfo.dls_size, 1), NULL);
    assert_not_do(dlinfo(handle, RTLD_DI_SERINFOSIZE, sip), -1, free(sip));
    assert_not_do(dlinfo(handle, RTLD_DI_SERINFO, sip), -1, free(sip));

    for (size_t i = 0; i < sip->dls_cnt; i++) {
        printf("%10s path %zu is %s\n",
               name, i, sip->dls_serpath[i].dls_name);
    }
    free(sip);
    return;
}

void* checkdl(const char* restrict name, const char* restrict path)
{
    void* dlhandle = dlopen(path, RTLD_NOLOAD);

    printf("%10s already there: %s\n",
           name, dlhandle == NULL ? "no" : "yes");
    if (dlhandle) dlclose(dlhandle);

    dlhandle = dlopen(path, RTLD_NOW);
    printf("%10s now     there: %s\n",
           name, dlhandle == NULL ? "no" : "yes");

    if (path && dlhandle) {
        printinfo(name, dlhandle);
    }
    return dlhandle;
}


int main(void)
{

    void* self = checkdl("Self", NULL);
    void* libc = checkdl("libc", "libc");
    void* libcso = checkdl("libc so", "libc.so");
    void* libcso6 = checkdl("libc so6", "libc.so.6");

    return EX_OK;
}
