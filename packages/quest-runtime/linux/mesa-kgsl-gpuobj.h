// SPDX-License-Identifier: MIT
// Quest 3 vendor SELinux limits app domains to an allowlist of KGSL ioctls
// (vendor_sepolicy.cil: allowx base_typeattr_650 gpu_device). It permits the
// GPUOBJ family (0x45-0x4a) but not the legacy GPUMEM_ALLOC_ID, FREE_ID and
// GET_INFO (0x34-0x36), which fail with EACCES and surface as Turnip's
// "BO init (VK_ERROR_OUT_OF_DEVICE_MEMORY)". Route them through GPUOBJ.
// Both families allocate the same KGSL mem entries, so Quest 2 is unaffected.

static int
quest_gpumem_alloc_id(int fd, struct kgsl_gpumem_alloc_id *req)
{
   struct kgsl_gpuobj_alloc alloc = {
      .size = req->size,
      .flags = req->flags,
   };
   if (safe_ioctl(fd, IOCTL_KGSL_GPUOBJ_ALLOC, &alloc))
      return -1;

   /* GPUOBJ_ALLOC does not return the GPU address. */
   struct kgsl_gpuobj_info info = { .id = alloc.id };
   if (safe_ioctl(fd, IOCTL_KGSL_GPUOBJ_INFO, &info)) {
      int err = errno;
      struct kgsl_gpuobj_free free_req = { .id = alloc.id };
      safe_ioctl(fd, IOCTL_KGSL_GPUOBJ_FREE, &free_req);
      errno = err;
      return -1;
   }

   /* The kernel echoes the flags it actually applied; callers probe on it. */
   req->id = alloc.id;
   req->flags = (unsigned int) alloc.flags;
   req->mmapsize = alloc.mmapsize;
   req->gpuaddr = info.gpuaddr;
   return 0;
}

static int
quest_gpumem_free_id(int fd, struct kgsl_gpumem_free_id *req)
{
   struct kgsl_gpuobj_free free_req = { .id = req->id };
   return safe_ioctl(fd, IOCTL_KGSL_GPUOBJ_FREE, &free_req);
}

static int
quest_gpumem_get_info(int fd, struct kgsl_gpumem_get_info *req)
{
   struct kgsl_gpuobj_info info = { .id = req->id };
   if (safe_ioctl(fd, IOCTL_KGSL_GPUOBJ_INFO, &info))
      return -1;

   req->gpuaddr = info.gpuaddr;
   req->flags = (unsigned int) info.flags;
   req->size = info.size;
   req->mmapsize = info.va_len;
   req->useraddr = info.va_addr;
   return 0;
}
