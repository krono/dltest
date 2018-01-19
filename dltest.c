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

void checksym(const char* restrict symbol, void* handle)
{
    printf("\t\t [in %p] ", handle);

    void* fun = dlsym(handle, symbol);
    if (fun) { // such fun
        Dl_info info;
        if (!dladdr(fun, &info)) {
            printf("no info found on %s\n", symbol);
            return;
        }
        if (info.dli_saddr == NULL) {
            printf("%s somehow not really found \n", symbol);
            return;
        }
        printf("%s[%p] resides in %s\n", symbol, fun, info.dli_fname);
    } else {
        printf("%s not found here\n", symbol);
    }
}

void checksym_and_close(void* handle)
{
    if (handle) {
        checksym("printf", handle);
        checksym("lseek", handle);
        dlclose(handle);
    }
}

int main(void)
{

    checksym("printf", RTLD_DEFAULT);
    checksym("lseek", RTLD_DEFAULT);

    void* self = checkdl("Self", NULL);
    checksym_and_close(self);

    void* libc = checkdl("libc", "libc");
    checksym_and_close(libc);

    void* libcso = checkdl("libc so", "libc.so");
    checksym_and_close(libcso);

    void* libcso6 = checkdl("libc so6", "libc.so.6");
    checksym_and_close(libcso6);


    return EX_OK;
}
