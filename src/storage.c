/**
 * @file storage.c
 * @brief Storage enumeration and file I/O operations
 */

#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/appmgr.h>
#include <string.h>
#include <stdio.h>

#include "storage.h"

/* Known storage paths on PS Vita */
static const struct
{
    const char *path;
    const char *name;
} g_known_storage[] = {
    {"ux0:", "Memory Card"},
    {"uma0:", "USB Storage"},
    {"imc0:", "Internal Memory"},
    {"xmc0:", "External Memory"},
    {NULL, NULL}};

int f3v_enumerate_storage(StorageDevice *devices, int max_devices)
{
    int count = 0;

    for (int i = 0; g_known_storage[i].path != NULL && count < max_devices; i++)
    {
        /* Check if path exists and is accessible */
        SceUID dir = sceIoDopen(g_known_storage[i].path);
        if (dir >= 0)
        {
            sceIoDclose(dir);

            /* Copy path and name */
            strncpy(devices[count].path, g_known_storage[i].path, sizeof(devices[count].path) - 1);
            strncpy(devices[count].name, g_known_storage[i].name, sizeof(devices[count].name) - 1);

            /* Get storage info */
            if (f3v_get_storage_info(&devices[count]) >= 0)
            {
                devices[count].writable = 1; /* Assume writable if we can open it */
                count++;
            }
        }
    }

    return count;
}

int f3v_get_storage_info(StorageDevice *device)
{
    uint64_t free_size = 0;
    uint64_t max_size = 0;

    /* Use sceAppMgrGetDevInfo to get storage info */
    int ret = sceAppMgrGetDevInfo(device->path, &max_size, &free_size);
    if (ret < 0)
    {
        return ret;
    }

    device->total_bytes = max_size;
    device->free_bytes = free_size;

    return 0;
}

int f3v_create_test_dir(TestContext *ctx)
{
    /* Build test directory path */
    snprintf(ctx->test_dir, sizeof(ctx->test_dir), "%s%s", ctx->target.path, F3V_TEST_DIR);

    /* Create parent directory (data/) if needed */
    char parent_dir[64];
    snprintf(parent_dir, sizeof(parent_dir), "%sdata", ctx->target.path);
    sceIoMkdir(parent_dir, 0777); /* Ignore error - might already exist */

    /* Create test directory */
    int ret = sceIoMkdir(ctx->test_dir, 0777);
    if (ret < 0 && ret != (int)0x80010011)
    { /* Ignore "already exists" error */
        return ret;
    }

    return 0;
}

char *f3v_get_test_filename(TestContext *ctx, uint32_t index, char *buf, size_t buf_size)
{
    snprintf(buf, buf_size, "%s/%s%03u%s",
             ctx->test_dir, F3V_FILE_PREFIX, index, F3V_FILE_EXT);
    return buf;
}

int f3v_open_write(const char *path)
{
    return sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
}

int f3v_open_read(const char *path)
{
    return sceIoOpen(path, SCE_O_RDONLY, 0);
}

int f3v_write_block(int fd, const void *buf, size_t size)
{
    return sceIoWrite(fd, buf, size);
}

int f3v_read_block(int fd, void *buf, size_t size)
{
    return sceIoRead(fd, buf, size);
}

int f3v_close(int fd)
{
    return sceIoClose(fd);
}

int f3v_cleanup_files(TestContext *ctx)
{
    int deleted = 0;
    char filename[128];

    /* Delete all test files */
    for (uint32_t i = 1; i <= ctx->files_written + 1; i++)
    {
        f3v_get_test_filename(ctx, i, filename, sizeof(filename));

        if (sceIoRemove(filename) >= 0)
        {
            deleted++;
        }
    }

    /* Try to remove the test directory (will fail if not empty) */
    sceIoRmdir(ctx->test_dir);

    return deleted;
}

int f3v_has_space(TestContext *ctx)
{
    /* Refresh storage info */
    f3v_get_storage_info(&ctx->target);

    /* Check if we have at least one block worth of free space */
    return (ctx->target.free_bytes >= F3V_BLOCK_SIZE);
}