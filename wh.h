/*
 * Copyright (c) 2016--2019  Wu, Xingbo <wuxb45@gmail.com>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// kv {{{
/*
 * Some internal union names can be ignored:
 * struct kv {
 *   u32 klen;
 *   u32 vlen;
 *   u64 hash;
 *   u8 kv[];
 * };
 */
struct kv {
  union { // the first u64
    u64 kvlen;
    struct {
      u32 klen;
      union {
        u32 vlen;
        u32 refcnt;
      };
    };
  };
  union {
    u64 hash; // hashvalue of the key
    struct {
      u32 hashlo; // little endian
      u32 hashhi;
    };
  };
  u8 kv[];  // len(kv) == klen + vlen
} __attribute__((packed));

// sized buffer: for returning value only
struct sbuf {
  u32 len;
  u8 buf[];
};

  extern size_t
kv_size(const struct kv * const kv);

  extern size_t
kv_size_align(const struct kv * const kv, const u64 align);

  extern size_t
key_size(const struct kv * const key);

  extern size_t
key_size_align(const struct kv * const key, const u64 align);

  extern void
kv_update_hash(struct kv * const kv);

  extern void
kv_refill(struct kv * const kv, const void * const key, const u32 klen, const void * const value, const u32 vlen);

  extern void
kv_refill_str_str(struct kv * const kv, const char * const key, const char * const value);

  extern void
kv_refill_str_u64(struct kv * const kv, const char * const key, const u64 value);

  extern struct kv *
kv_create(const void * const key, const u32 klen, const void * const value, const u32 vlen);

  extern struct kv *
kv_create_str(const char * const key, const char * const value);

  extern struct kv *
kv_dup(const struct kv * const kv);

  extern struct kv *
kv_dup_key(const struct kv * const kv);

  extern struct kv *
kv_dup2(const struct kv * const from, struct kv * const to);

  extern struct kv *
kv_dup2_key(const struct kv * const from, struct kv * const to);

  extern struct kv *
kv_dup2_key_prefix(const struct kv * const from, struct kv * const to, const u64 plen);

  extern struct sbuf *
kv_dup2_sbuf(const struct kv * const from, struct sbuf * const to);

  extern struct kv *
kv_alloc_malloc(const u64 size, void * const priv);

  extern void
kv_retire_free(struct kv * const kv, void * const priv);

  extern bool
kv_keymatch(const struct kv * const key1, const struct kv * const key2);

  extern bool
kv_keymatch_r(const struct kv * const key1, const struct kv * const key2);

  extern bool
kv_fullmatch(const struct kv * const kv1, const struct kv * const kv2);

typedef int  (*kv_compare_func)(const struct kv * const kv1, const struct kv * const kv2);

  extern int
kv_keycompare(const struct kv * const kv1, const struct kv * const kv2);

  extern int
kv_compare_vi128(const struct kv * const sk, const u8 * const vi128);

  extern void
kv_qsort(const struct kv ** const kvs, const size_t nr);

  extern void *
kv_vptr(struct kv * const kv);

  extern void *
kv_kptr(struct kv * const kv);

  extern const void *
kv_vptr_c(const struct kv * const kv);

  extern const void *
kv_kptr_c(const struct kv * const kv);

  extern u32
kv_key_lcp(const struct kv * const key1, const struct kv * const key2);

  extern void
kv_print(const struct kv * const kv, const char * const cmd, FILE * const out);

  extern size_t
kv_vi128_estimate(const struct kv * const kv);

  extern u8 *
kv_vi128_encode(u8 * ptr, const struct kv * const kv);
// }}} kv

// kvmap {{{
typedef void (* kv_inplace_func)(struct kv * const curr, void * const priv);

typedef struct kv * (* kv_alloc_func)(const u64, void * const);

typedef void (* kv_retire_func)(struct kv * const, void * const);

struct kvmap_mm {
  kv_alloc_func af;
  void * ap;
  kv_retire_func rf;
  void * rp;
};
// }}} kvmap

// wormhole {{{
struct wormhole;
struct wormref;

// the wh created by wormhole_create() can work with all of safe/unsafe operations.
  extern struct wormhole *
wormhole_create(const struct kvmap_mm * const mm);

// the wh created by whunsafe_create() can only work with the unsafe operations.
  extern struct wormhole *
whunsafe_create(const struct kvmap_mm * const mm);

  extern struct kv *
wormhole_get(struct wormref * const ref, const struct kv * const key, struct kv * const out);

  extern bool
wormhole_probe(struct wormref * const ref, const struct kv * const key);

  extern bool
wormhole_set(struct wormref * const ref, const struct kv * const kv);

  extern bool
wormhole_inplace(struct wormref * const ref, const struct kv * const key,
    kv_inplace_func uf, void * const priv);

  extern bool
wormhole_del(struct wormref * const ref, const struct kv * const key);

  extern struct wormhole_iter *
wormhole_iter_create(struct wormref * const ref);

  extern void
wormhole_iter_seek(struct wormhole_iter * const iter, const struct kv * const key);

  extern struct kv *
wormhole_iter_peek(struct wormhole_iter * const iter, struct kv * const out);

  extern struct kv *
wormhole_iter_next(struct wormhole_iter * const iter, struct kv * const out);

  extern void
wormhole_iter_skip(struct wormhole_iter * const iter, const u64 nr);

  extern bool
wormhole_iter_inplace(struct wormhole_iter * const iter, kv_inplace_func uf, void * const priv);

  extern void
wormhole_iter_destroy(struct wormhole_iter * const iter);

  extern struct wormref *
wormhole_ref(struct wormhole * const map);

  extern struct wormhole *
wormhole_unref(struct wormref * const ref);

  extern void
wormhole_refresh_qstate(struct wormref * const ref);

  extern void
wormhole_clean(struct wormhole * const map);

  extern void
wormhole_destroy(struct wormhole * const map);

// unsafe API

  extern struct kv *
whunsafe_get(struct wormhole * const map, const struct kv * const key, struct kv * const out);

  extern bool
whunsafe_probe(struct wormhole * const map, const struct kv * const key);

  extern bool
whunsafe_set(struct wormhole * const map, const struct kv * const kv);

  extern bool
whunsafe_inplace(struct wormhole * const map, const struct kv * const key,
    kv_inplace_func uf, void * const priv);

  extern bool
whunsafe_del(struct wormhole * const map, const struct kv * const key);

  extern struct wormhole_iter *
whunsafe_iter_create(struct wormhole * const map);

  extern void
whunsafe_iter_seek(struct wormhole_iter * const iter, const struct kv * const key);

  extern struct kv *
whunsafe_iter_peek(struct wormhole_iter * const iter, struct kv * const out);

  extern struct kv *
whunsafe_iter_next(struct wormhole_iter * const iter, struct kv * const out);

  extern void
whunsafe_iter_skip(struct wormhole_iter * const iter, const u64 nr);

  extern bool
whunsafe_iter_inplace(struct wormhole_iter * const iter, kv_inplace_func uf, void * const priv);

  extern void
whunsafe_iter_destroy(struct wormhole_iter * const iter);

  extern bool
wormhole_locking(struct wormhole * const map, const bool locking);

  extern bool
whunsafe_locking(struct wormhole * const map, const bool locking);

  extern void
wormhole_fprint(struct wormhole * const map, FILE * const out);

// verify & debugging
  extern bool
wormhole_verify(struct wormhole * const map);

  extern void
wormhole_dump_memory(struct wormhole * const map, const char * const filename, const char * const opt);

  extern bool
wormhole_merge_at(struct wormref * const ref, const struct kv * const key);

  extern bool
wormhole_split_at(struct wormref * const ref, const struct kv * const key);

  extern void
wormhole_sync_at(struct wormref * const ref, const struct kv * const key);

  extern void
wormhole_print_meta_anchors(struct wormhole * const map, const char * const pattern);

  extern void
wormhole_print_leaf_anchors(struct wormhole * const map, const char * const pattern);

  extern void
wormhole_print_meta_lrmost(struct wormhole * const map, const char * const pattern);

  extern void *
wormhole_jump_leaf_only(struct wormhole * const map, const struct kv * const key);

  extern struct kv **
wormhole_anchors(struct wormhole * const map);
// }}} wormhole

#ifdef __cplusplus
}
#endif
// vim:fdm=marker
