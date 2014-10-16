/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
#ifndef _SU_SRV_PFS_H
#define _SU_SRV_PFS_H

#ifdef __cplusplus
extern "C" {
#endif

/// su_srv_init_pfs(): Initializes minimal private filesystem.
/// 
/// Creates <pfs_root>/var/log and <pfs_root>/var/run.
///
/// Returns: 0 on success, errno on failure. 
///
int su_srv_pfs_init(const char *pfs_root);

#ifdef __cplusplus
}
#endif

#endif
