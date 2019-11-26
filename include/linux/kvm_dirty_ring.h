#ifndef KVM_DIRTY_RING_H
#define KVM_DIRTY_RING_H

/*
 * struct kvm_dirty_ring is defined in include/uapi/linux/kvm.h.
 *
 * dirty_ring:  shared with userspace via mmap. It is the compact list
 *              that holds the dirty pages.
 * dirty_index: free running counter that points to the next slot in
 *              dirty_ring->dirty_gfns  where a new dirty page should go.
 * reset_index: free running counter that points to the next dirty page
 *              in dirty_ring->dirty_gfns for which dirty trap needs to
 *              be reenabled
 * size:        size of the compact list, dirty_ring->dirty_gfns
 * soft_limit:  when the number of dirty pages in the list reaches this
 *              limit, vcpu that owns this ring should exit to userspace
 *              to allow userspace to harvest all the dirty pages
 * lock:        protects dirty_ring, only in use if this is the global
 *              ring
 *
 * The number of dirty pages in the ring is calculated by,
 * dirty_index - reset_index
 *
 * kernel increments dirty_ring->indices.avail_index after dirty index
 * is incremented. When userspace harvests the dirty pages, it increments
 * dirty_ring->indices.fetch_index up to dirty_ring->indices.avail_index.
 * When kernel reenables dirty traps for the dirty pages, it increments
 * reset_index up to dirty_ring->indices.fetch_index.
 *
 */
struct kvm_dirty_ring {
	u16 dirty_index;
	u16 reset_index;
	u32 size;
	u32 soft_limit;
	spinlock_t lock;
	struct kvm_dirty_gfn *dirty_gfns;
};

u32 kvm_dirty_ring_get_rsvd_entries(void);
int kvm_dirty_ring_alloc(struct kvm_vcpu *vcpu);

/*
 * called with kvm->slots_lock held, returns the number of
 * processed pages.
 */
int kvm_dirty_ring_reset(struct kvm_vcpu *vcpu);

/*
 * returns 0: successfully pushed
 *         1: successfully pushed, soft limit reached,
 *            vcpu should exit to userspace
 *         -EBUSY: unable to push, dirty ring full.
 */
int kvm_dirty_ring_push(struct kvm_vcpu *vcpu, u32 slot, u64 offset);

/* for use in vm_operations_struct */
struct page *kvm_dirty_ring_get_page(struct kvm_vcpu *vcpu, u32 i);

void kvm_dirty_ring_free(struct kvm_vcpu *vcpu);

#endif
