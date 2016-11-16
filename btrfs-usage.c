#include <btrfs/ctree.h>
#include <btrfs/ioctl.h>
#include <dirent.h>
#include <errno.h>
#include <linux/magic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>

struct btrfs_ioctl_space_args *load_space_info(int fd, const char *path) {
  struct btrfs_ioctl_space_args sargs;
  memset(&sargs, 0, sizeof(sargs));
  if (ioctl(fd, BTRFS_IOC_SPACE_INFO, &sargs) < 0) {
    fprintf(stderr, "ioctl(%s, BTRFS_IOC_SPACE_INFO): %s\n", path, strerror(errno));
    return NULL;
  }

  struct btrfs_ioctl_space_args *sargs_with_info =
      malloc(sizeof(*sargs_with_info) +
             sargs.total_spaces * sizeof(*sargs_with_info->spaces));
  sargs_with_info->space_slots = sargs.total_spaces;
  sargs_with_info->total_spaces = 0;
  if (ioctl(fd, BTRFS_IOC_SPACE_INFO, sargs_with_info) < 0) {
    fprintf(stderr, "ioctl(%s, BTRFS_IOC_SPACE_INFO): %s\n", path, strerror(errno));
    return NULL;
  }

  return sargs_with_info;
}

static int show_filesystem_usage(const char *path) {
  DIR *dir = opendir(path);
  if (dir == NULL) {
    fprintf(stderr, "opendir(%s): %s\n", path, strerror(errno));
    return 1;
  }

  int fd = dirfd(dir);
  if (fd == -1) {
    fprintf(stderr, "dirfd(%s): %s\n", path, strerror(errno));
    return 1;
  }

  struct statfs sfs;
  if (fstatfs(fd, &sfs) < 0) {
    fprintf(stderr, "fstatfs(%s): %s\n", path, strerror(errno));
    return 1;
  }
  if (sfs.f_type != BTRFS_SUPER_MAGIC) {
    fprintf(stderr, "not a btrfs filesystem: %s\n", path);
    return 1;
  }

  struct btrfs_ioctl_fs_info_args fi_args;
  if (ioctl(fd, BTRFS_IOC_FS_INFO, &fi_args) < 0) {
    fprintf(stderr, "ioctl(%s, BTRFS_IOC_FS_INFO): %s\n", path,
            strerror(errno));
    return 1;
  }

  __u64 device_size = 0;
  for (__u64 i = 0; i <= fi_args.max_id; i++) {
    struct btrfs_ioctl_dev_info_args di_args = {0};
    di_args.devid = i;
    if (ioctl(fd, BTRFS_IOC_DEV_INFO, &di_args) < 0) {
      if (errno == ENODEV) {
        continue;
      }
      fprintf(stderr, "ioctl(%s, BTRFS_IOC_DEV_INFO): %s\n", path,
              strerror(errno));
    }
    device_size += di_args.total_bytes;
  }

  struct btrfs_ioctl_space_args *sargs = load_space_info(fd, path);
  if (sargs == NULL) {
    return 1;
  }

  __u64 data_used = 0, metadata_used = 0, system_used = 0;
  __u64 data_chunks = 0, metadata_chunks = 0, system_chunks = 0;
  for (__u64 i = 0; i < sargs->total_spaces; i++) {
    const struct btrfs_ioctl_space_info *space = sargs->spaces + i;
    const __u64 flags = space->flags;

    if (flags & BTRFS_BLOCK_GROUP_DATA) {
      data_used += space->used_bytes;
      data_chunks += space->total_bytes;
    }
    if (flags & BTRFS_BLOCK_GROUP_METADATA) {
      metadata_used += space->used_bytes;
      metadata_chunks += space->total_bytes;
    }
    if (flags & BTRFS_BLOCK_GROUP_SYSTEM) {
      system_used += space->used_bytes;
      system_chunks += space->total_bytes;
    }
  }

  printf(
      "{\"size\":%llu, \"allocated\":{\"data\":%llu, \"metadata\":%llu, "
      "\"system\":%llu}, "
      "\"used\":{\"data\":%llu, \"metadata\":%llu, \"system\":%llu}}\n",
      device_size, data_chunks, metadata_chunks, system_chunks, data_used,
      metadata_used, system_used);

  free(sargs);
  closedir(dir);

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s /path/to/mount/point\n", argv[0]);
    return 2;
  }

  return show_filesystem_usage(argv[1]);
}
